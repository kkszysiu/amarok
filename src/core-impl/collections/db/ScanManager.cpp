/****************************************************************************************
 * Copyright (c) 2003-2008 Mark Kretschmann <kretschmann@kde.org>                       *
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 * Copyright (c) 2007 Casey Link <unnamedrambler@gmail.com>                             *
 * Copyright (c) 2008 Leo Franchi <lfranchi@kde.org>                                    *
 * Copyright (c) 2008-2009 Jeff Mitchell <mitchell@kde.org>                             *
 * Copyright (c) 2010-2011 Ralf Engels <ralf-engels@gmx.de>                             *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#define DEBUG_PREFIX "ScanManager"

#include "core-impl/collections/db/ScanManager.h"

#include "App.h"
#include "MountPointManager.h"
#include "ScanResultProcessor.h"
#include "amarokconfig.h"
#include "core/support/Debug.h"
#include "sql/SqlCollection.h"
#include "statusbar/StatusBar.h"

// include files from the collection scanner utility
#include <collectionscanner/BatchFile.h>
#include <collectionscanner/Directory.h>

#include <QFileInfo>
#include <QSharedMemory>

#include "MainWindow.h"
#include <KMessageBox>
#include <KStandardDirs>
#include <KDirWatch>

#include <threadweaver/ThreadWeaver.h>

static const int MAX_RESTARTS = 40;
static const int WATCH_INTERVAL = 60 * 1000; // = 60 seconds

static const int DELAYED_SCAN_INTERVAL = 2 * 1000; // = 2 seconds

static const int SHARED_MEMORY_SIZE = 1024 * 1024; // 1 MB shared memory

ScanManager::ScanManager( Collections::DatabaseCollection *collection, QObject *parent )
    : QObject( parent )
    , m_collection( static_cast<Collections::SqlCollection*>( collection ) )
    , m_scanner( 0 )
    , m_errorsReported( false )
    , m_blockCount( 0 )
    , m_fullScanRequested( false )
    , m_importRequested( 0 )
    , m_watcherJob( 0 )
    , m_checkDirsTimer( 0 )
    , m_delayedScanTimer( 0 )
    , m_mutex( QMutex::Recursive )
{
    // -- check the scanner version timer
    QTimer::singleShot( WATCH_INTERVAL / 2, this, SLOT( checkScannerVersion() ) );

    // -- check the scanner configuration timer
    m_checkDirsTimer = new QTimer( this );
    connect( m_checkDirsTimer, SIGNAL( timeout() ), this, SLOT( checkForDirectoryChanges() ) );
    m_checkDirsTimer->start( WATCH_INTERVAL );

    // -- delayed scan timer
    m_delayedScanTimer = new QTimer( this );
    m_delayedScanTimer->setSingleShot( true );
    connect( m_delayedScanTimer, SIGNAL( timeout() ), this, SLOT( startScanner() ) );
}

ScanManager::~ScanManager()
{
    abort();
    // actually we should wait for the abort to be finished.
    delete m_importRequested;
}

void
ScanManager::blockScan()
{
    {
        QMutexLocker locker( &m_mutex );

        m_blockCount ++;
    }
    abort( "Scan blocked" );
}

void
ScanManager::unblockScan()
{
    {
        QMutexLocker locker( &m_mutex );
        m_blockCount --;
    }
    startScanner();
}

bool
ScanManager::isRunning()
{
    return m_scanner;
}


void
ScanManager::requestFullScan()
{
    {
        QMutexLocker locker( &m_mutex );
        m_fullScanRequested = true;
        m_scanDirsRequested.unite( m_collection->mountPointManager()->collectionFolders().toSet() );

        if( m_scanDirsRequested.isEmpty() )
            warning() << "Cleaning complete collection requested.";
    }
    startScanner();
}

void
ScanManager::requestImport( QIODevice *input )
{
    abort( i18n("Database import requested") );
    {
        QMutexLocker locker( &m_mutex );
        m_fullScanRequested = false;
        m_scanDirsRequested.clear();
        m_importRequested = input;
    }
    startScanner();
}


void
ScanManager::requestIncrementalScan( const QString &directory )
{
    DEBUG_BLOCK;
    addDirToList( directory );
    startScanner();
}


void
ScanManager::delayedIncrementalScan( const QString &directory )
{
    DEBUG_BLOCK;
    addDirToList( directory );
    m_delayedScanTimer->start( DELAYED_SCAN_INTERVAL );
}


void
ScanManager::delayedIncrementalScanParent( const QString &directory )
{
    DEBUG_BLOCK;
    addDirToList( QFileInfo( directory ).path() );
    m_delayedScanTimer->start( DELAYED_SCAN_INTERVAL );
}


void
ScanManager::addDirToList( const QString &directory )
{
    QMutexLocker locker( &m_mutex );
    debug() << "addDirToList for"<<directory;

    if( directory.isEmpty() )
        m_scanDirsRequested.unite( m_collection->mountPointManager()->collectionFolders().toSet() );
    else
    {
        if( m_collection->isDirInCollection( directory ) )
            m_scanDirsRequested.insert( directory );
        else
        {
            ; // the CollectionManager asks every collection for the scan. No harm done.
        }
    }
}

void
ScanManager::startScanner()
{
    QMutexLocker locker( &m_mutex );

    if( !m_fullScanRequested && m_scanDirsRequested.isEmpty() && !m_importRequested )
        return; // nothing to do

    if( isRunning() )
    {
        debug() << "scanner already running";
        return;
    }
    if( m_blockCount > 0 )
    {
        debug() << "scanning currently blocked";
        return;
    }
    if( pApp && pApp->isNonUniqueInstance() )
        warning() << "Running scanner from Amarok while another Amarok instance is also running is dangerous.";

    // -- create the scanner job
    if( m_importRequested )
    {
        m_scanner = new ScannerJob( this, m_collection, m_importRequested );

        m_importRequested = 0;
    }
    else
    {
        // - what kind of scan are we actually doing?
        ScanResultProcessor::ScanType scanType;
        if( m_fullScanRequested )
            scanType = ScanResultProcessor::FullScan;
        else if( m_scanDirsRequested == m_collection->mountPointManager()->collectionFolders().toSet() )
            scanType = ScanResultProcessor::UpdateScan;
        else
            scanType = ScanResultProcessor::PartialUpdateScan;

        m_scanner = new ScannerJob( this, m_collection, scanType,
                                    m_scanDirsRequested.toList() );

        m_fullScanRequested = false;
        m_scanDirsRequested.clear();
    }

    // - connect the status bar
    if( The::statusBar() )
    {
        The::statusBar()->newProgressOperation( m_scanner, i18n( "Scanning music" ) )
            ->setAbortSlot( this, SLOT( abort() ) );

        connect( m_scanner, SIGNAL( totalSteps(const QObject*, int) ),
                 The::statusBar(), SLOT( setProgressTotalSteps(const QObject*, int) ),
                 Qt::QueuedConnection );
        connect( m_scanner, SIGNAL( step(const QObject*) ),
                 The::statusBar(), SLOT( incrementProgress(const QObject*) ),
                 Qt::QueuedConnection );
    }

    // - enqueue it.
    connect( m_scanner, SIGNAL( done( ThreadWeaver::Job* ) ), SLOT( slotJobDone() ) );
    connect( m_scanner, SIGNAL( message( QString ) ), this, SIGNAL( message( QString ) ) );
    connect( m_scanner, SIGNAL( succeeded() ), this, SIGNAL( succeeded() ) );
    connect( m_scanner, SIGNAL( failed( QString ) ), this, SIGNAL( failed( QString ) ) );
    ThreadWeaver::Weaver::instance()->enqueue( m_scanner );

}

void
ScanManager::checkScannerVersion()
{
    DEBUG_BLOCK;

    if( !AmarokConfig::monitorChanges() )
        return;

    // upps. that blocks. Hopefully the scanner starts fast
    QProcess scanner;
    scanner.start( ScannerJob::scannerPath(), QStringList( "--version" ) );
    scanner.waitForFinished();

    const QString version = scanner.readAllStandardOutput().trimmed();
    if( version != AMAROK_VERSION  )
    {
        KMessageBox::error( 0, i18n( "<p>The version of the 'amarokcollectionscanner' tool\n"
                                     "does not match your Amarok version.</p>"
                                     "<p>Please note that Collection Scanning may not work correctly.</p>" ) );
    }
}


void
ScanManager::checkForDirectoryChanges()
{
    DEBUG_BLOCK;

    if( !AmarokConfig::monitorChanges() )
    {
        if( m_watcherJob )
            m_watcherJob->setPaused( true );
        return;
    }
    else
    {
        // TODO: re-create the watcher if scanRecursively has changed
        if( m_watcherJob )
        {
            m_watcherJob->setPaused( false );
        }
        else
        {
            // -- Check if directories changed while we didn't have a watcher
            requestIncrementalScan();

            m_watcherJob = new DirWatchJob( this, m_collection );
            ThreadWeaver::Weaver::instance()->enqueue( m_watcherJob );
        }
    }
}

void
ScanManager::abort( const QString &reason )
{
    QMutexLocker locker( &m_mutex );

    if( !reason.isEmpty() )
        debug() << "Abort scan: " << reason;

    if( m_scanner )
        m_scanner->requestAbort( reason );
    // TODO: For testing it would be good to specify the scanner in the build directory
}


void
ScanManager::slotJobDone()
{
    QMutexLocker locker( &m_mutex );

    if( m_scanner )
    {
        // -- error reporting
        if( !m_errorsReported )
        {
            m_errorsReported = true;

            if( !m_scanner->getLastErrors().isEmpty() )
            {
                KMessageBox::error( The::mainWindow(), //parent
                                    i18n( "The collection scanner reported the following errors:\n"
                                          "%1\n"
                                          "In most cases this means that not all of your tracks "
                                          "were imported.\n"
                                          "Further errors will only be reported on the console." ).
                                    arg(m_scanner->getLastErrors().join("\n")) );
            }
        }

        m_scanner->deleteLater();
        m_scanner = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class DirWatchJob
///////////////////////////////////////////////////////////////////////////////

DirWatchJob::DirWatchJob( QObject *parent, Collections::DatabaseCollection *collection )
    : ThreadWeaver::Job( parent )
    , m_collection( collection )
{
    DEBUG_BLOCK;

    // -- create a new watcher
    m_watcher = new KDirWatch( this );

    connect( m_watcher, SIGNAL( dirty(const QString &) ),
             parent, SLOT( delayedIncrementalScan(const QString &) ) );
    connect( m_watcher, SIGNAL( created(const QString &) ),
             parent, SLOT( delayedIncrementalScanParent(const QString &) ) );
    connect( m_watcher, SIGNAL( deleted(const QString &) ),
             parent, SLOT( delayedIncrementalScanParent(const QString &) ) );

    m_watcher->startScan( false );
}

void
DirWatchJob::run()
{
    DEBUG_BLOCK;

    // -- update the KDirWatch with the current set of directories
    QSet<QString> dirs = m_collection->mountPointManager()->collectionFolders().toSet();

    // - add new
    QSet<QString> newDirs = dirs - m_oldWatchDirs;
    foreach( const QString& dir, newDirs )
    {
        m_watcher->addDir( dir,
                           AmarokConfig::scanRecursively() ?
                           KDirWatch::WatchSubDirs : KDirWatch::WatchDirOnly );
    }

    // - remove old
    QSet<QString> removeDirs = m_oldWatchDirs - dirs;
    foreach( const QString& dir, removeDirs )
    {
        m_watcher->removeDir( dir );
    }

    m_oldWatchDirs = dirs;
}

void
DirWatchJob::setPaused( bool pause )
{
    DEBUG_BLOCK;

    if( pause && !m_watcher->isStopped() )
        m_watcher->stopScan();
    else if( !pause && m_watcher->isStopped() )
        m_watcher->startScan( true );
}


///////////////////////////////////////////////////////////////////////////////
// class ScannerJob
///////////////////////////////////////////////////////////////////////////////

ScannerJob::ScannerJob( QObject *parent, Collections::DatabaseCollection *collection,
                          QIODevice *input )
    : ThreadWeaver::Job( parent )
    , m_collection( collection )
    , m_scanType( ScanResultProcessor::FullScan )
    , m_input( input )
    , m_restartCount( 0 )
    , m_abortRequested( false )
    , m_scanner( 0 )
    , m_scannerStateMemory( 0 )
{
    Q_ASSERT( m_input );
}

ScannerJob::ScannerJob( QObject *parent, Collections::DatabaseCollection *collection,
                          ScanResultProcessor::ScanType scanType,
                          QStringList scanDirsRequested )
    : ThreadWeaver::Job( parent )
    , m_collection( collection )
    , m_scanType( scanType )
    , m_scanDirsRequested( scanDirsRequested )
    , m_restartCount( 0 )
    , m_abortRequested( false )
    , m_scanner( 0 )
    , m_scannerStateMemory( 0 )
{ }

ScannerJob::~ScannerJob()
{
    DEBUG_BLOCK;
    QMutexLocker locker( &m_mutex );

    // remove the batch file
    if( !m_batchfilePath.isEmpty() )
        QFile::remove( m_batchfilePath );

    delete m_scannerStateMemory;
}

void
ScannerJob::run()
{
    DEBUG_BLOCK;

    // -- initialize the input
    // - from io device
    if( m_input )
    {
        m_reader.setDevice( m_input.data() );
    }
    // - from process
    else
    {
        // -- write the batch file
        m_batchfilePath = KGlobal::dirs()->saveLocation( "data", QString("amarok/"), false ) + "amarokcollectionscanner_batchscan.xml";

        while( QFile::exists( m_batchfilePath ) )
            m_batchfilePath += "_";

        CollectionScanner::BatchFile batchfile;
        batchfile.setTimeDefinitions( getKnownDirs() );
        batchfile.setDirectories( m_scanDirsRequested );
        if( !batchfile.write( m_batchfilePath ) )
        {
            warning() << "Failed to write file" << m_batchfilePath;
            emit failed( i18n("Failed to write scanner batch file \"%1\"").arg( m_batchfilePath) );
            m_batchfilePath.clear();
            return;
        }

        if( !createScannerProcess( false ) )
        {
            warning() << "Unable to start Amarok colleciton scanner.";
            emit failed( i18n("Unable to start Amarok collection scanner." ) );
            return;
        }
    }

    // -- create the result processor
    ScanResultProcessor *processor = m_collection->getNewScanResultProcessor();
    processor->setType( m_scanType );
    connect( processor, SIGNAL( directoryCommitted() ), this, SLOT( directoryProcessed() ) );
    connect( processor, SIGNAL( directorySkipped() ), this, SLOT( directoryProcessed() ) );

    // -- read the input and loop
    bool finished = false;
    int count = 0;
    do
    {
        // -- check if we were aborted, have finished or need to wait for new data
        {
            QMutexLocker locker( &m_mutex );
            if( m_abortRequested )
                break;
        }

        if( m_scanner && m_reader.atEnd() )
            getScannerOutput();

        if( m_scanner && m_scanner->exitStatus() != QProcess::NormalExit && !tryRestart() )
            break;

        // -- scan as many directory tags as we added to the data
        while( !m_reader.atEnd() )
        {
            m_reader.readNext();
            if( m_reader.hasError() )
            {
                break;
            }
            else if( m_reader.isStartElement() )
            {
                QStringRef name = m_reader.name();
                if( name == "scanner" )
                {
                    // when importing we can only tell an incremental scan from the
                    // way the scanner was started
                    if( m_input &&
                        m_reader.attributes().hasAttribute("incremental") )
                        processor->setType( ScanResultProcessor::PartialUpdateScan );

                    debug() << "ScannerJob: got count:" << m_reader.attributes().value( "count" ).toString().toInt();
                    emit message( i18np("Found one directory", "Found %1 directories",
                                  m_reader.attributes().value( "count" ).toString()) );
                    emit totalSteps( this,
                                     m_reader.attributes().value( "count" ).toString().toInt() * 2);
                }
                else if( name == "directory" )
                {
                    CollectionScanner::Directory *dir = new CollectionScanner::Directory( &m_reader );
                    processor->addDirectory( dir );
                    debug() << "ScannerJob: run:"<<count<<"current path"<<dir->rpath();
                    count++;

                    emit message( i18n("Got directory \"%1\" from scanner.").arg( dir->rpath() ) );
                    emit step( this );
                }
                else
                {
                    warning() << "Unexpected xml start element"<<name<<"in input";
                    m_reader.skipCurrentElement();
                }

            }
            else if( m_reader.isEndElement() )
            {
                if( m_reader.name() == "scanner" ) // ok. finished
                    finished = true;
            }
            else if( m_reader.isEndDocument() )
            {
                finished = true;
            }
        }

    } while( !finished &&
             (!m_reader.hasError() || m_reader.error() == QXmlStreamReader::PrematureEndOfDocumentError) );

    if( m_scanner )
    {
        m_scanner->close();
        m_scanner->waitForFinished(); // waits at most 3 seconds
        delete m_scanner;
        m_scanner = 0;
    }

    if( m_abortRequested )
    {
        debug() << "Aborting ScanManager ScannerJob";
        emit failed( m_abortReason );
        processor->rollback();
    }
    else if( !finished && m_reader.hasError() )
    {
        warning() << "Aborting ScanManager ScannerJob with error"<<m_reader.errorString();
        emit failed( i18n("Aborting scanner with error: %1").arg( m_reader.errorString() ) );
        processor->rollback();
    }
    else
    {
        processor->commit();
        emit succeeded();
    }

    debug() << "ScannerJob finished";

    m_lastErrors = processor->getLastErrors();
    delete processor;
    processor = 0;
}

void
ScannerJob::requestAbort()
{
    requestAbort( i18n("Scanner aborted.") );
}

void
ScannerJob::requestAbort( const QString &reason )
{
    QMutexLocker locker( &m_mutex );
    m_abortRequested = true;
    m_abortReason = reason;
}

QString
ScannerJob::scannerPath()
{
    QString path = KStandardDirs::locate( "exe", "amarokcollectionscanner" );

    // If the binary is not in $PATH, then search in the application folder too
    if( path.isEmpty() )
        path = App::applicationDirPath() + QDir::separator() + "amarokcollectionscanner";

    return path;
}


void
ScannerJob::directoryProcessed()
{
    emit step( this );
}


bool
ScannerJob::createScannerProcess( bool restart )
{
    DEBUG_BLOCK;
    Q_ASSERT( !m_scanner );

    if( m_abortRequested )
        return false;

    // -- create the shared memory
    if( !m_scannerStateMemory && !restart )
    {
        m_sharedMemoryKey = "AmarokScannerMemory"+QDateTime::currentDateTime().toString();
        m_scannerStateMemory = new QSharedMemory( m_sharedMemoryKey );
        if( !m_scannerStateMemory->create( SHARED_MEMORY_SIZE ) )
        {
            warning() << "Unable to create shared memory for collection scanner";
            delete m_scannerStateMemory;
            m_scannerStateMemory = 0;
        }
    }

    // -- create the scanner process
    m_scanner = new AmarokProcess();
    m_scanner->setOutputChannelMode( KProcess::OnlyStdoutChannel );

    *m_scanner << ScannerJob::scannerPath() << "--idlepriority";
    if( m_scanType != ScanResultProcessor::FullScan )
        *m_scanner << "-i";

    if( AmarokConfig::scanRecursively() )
        *m_scanner << "-r";

    if( AmarokConfig::useCharsetDetector() )
        *m_scanner << "-c";

    if( restart )
        *m_scanner << "-s";

    if( m_scannerStateMemory )
        *m_scanner << "--sharedmemory" << m_sharedMemoryKey;

    *m_scanner << "--batch" << m_batchfilePath;

    debug() << "starting scanner now:";
    m_scanner->start();
    return m_scanner->waitForStarted( -1 );
}


bool
ScannerJob::tryRestart()
{
    DEBUG_BLOCK;

    if( m_scanner->exitStatus() == QProcess::NormalExit )
        return false; // all shiny. no need to restart

    m_restartCount++;
    debug() << "Collection scanner crashed, restart count is " << m_restartCount;

    delete m_scanner;
    m_scanner = 0;

    if( m_restartCount >= MAX_RESTARTS )
    {
        requestAbort( i18n("The collection scan had to be aborted. Too many errors were encountered during the scan." ));
        return false;
    }
    else
        return createScannerProcess( true );
}

void
ScannerJob::getScannerOutput()
{
    DEBUG_BLOCK;

    if( !m_scanner->waitForReadyRead( -1 ) )
        return;
    m_incompleteTagBuffer += m_scanner->readAll();

    int index = m_incompleteTagBuffer.lastIndexOf( "</scanner>" );
    if( index >= 0 )
    {
        // append new data (we need to be locked. the reader is probalby not thread save)
        m_reader.addData( m_incompleteTagBuffer.left( index + 10 ) );
        m_incompleteTagBuffer = m_incompleteTagBuffer.mid( index + 10 );
    }
    else
    {
        index = m_incompleteTagBuffer.lastIndexOf( "</directory>" );
        if( index >= 0 )
        {
            // append new data (we need to be locked. the reader is probalby not thread save)
            m_reader.addData( m_incompleteTagBuffer.left( index + 12 ) );
            m_incompleteTagBuffer = m_incompleteTagBuffer.mid( index + 12 );
        }
    }

}


QList< QPair<QString, uint> >
ScannerJob::getKnownDirs()
{
    QList< QPair<QString, uint> > result;

    // -- get all mount points
    QList<int> idlist = m_collection->mountPointManager()->getMountedDeviceIds();

    //expects a stringlist in order deviceid, dir, changedate
    QStringList values = m_collection->getDatabaseDirectories( idlist );
    for( QListIterator<QString> iter( values ); iter.hasNext(); )
    {
        int deviceid = iter.next().toInt();
        QString dir = iter.next();
        uint mtime = iter.next().toUInt();

        QString folder = m_collection->mountPointManager()->getAbsolutePath( deviceid, dir );
        result.append( QPair<QString, uint>( folder, mtime ) );
    }

    return result;
}


#include "ScanManager.moc"

