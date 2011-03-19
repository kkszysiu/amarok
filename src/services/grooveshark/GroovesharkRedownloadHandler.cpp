/****************************************************************************************
 * Copyright (c) 2006,2007 Nikolaj Hald Nielsen <nhn@kde.org>                           *
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

#include "GroovesharkRedownloadHandler.h"

#include "GroovesharkConfig.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"

#include <KLocale>
#include <KMessageBox>

#include "QDir"


GroovesharkRedownloadHandler::GroovesharkRedownloadHandler(QWidget * parent)
{
    m_parent = parent;
    m_redownloadDialog = 0;
    m_downloadDialog = 0;
    m_albumDownloader = 0;
}


GroovesharkRedownloadHandler::~GroovesharkRedownloadHandler()
{
}

void
GroovesharkRedownloadHandler::showRedownloadDialog( )
{
    fetchServerSideRedownloadList();
    return;
}

QStringList
GroovesharkRedownloadHandler::GetPurchaseList( )
{
   
    debug() << "GroovesharkRedownloadHandler::GetPurchaseList( )";
    
    QStringList returnList;
    QDir purchaseInfoDir( Amarok::saveLocation( "grooveshark.com/purchases/" ) );

    if ( !purchaseInfoDir.exists () ) {
      return returnList;
    }

    purchaseInfoDir.setFilter( QDir::Files);
    purchaseInfoDir.setSorting( QDir::Name );

    const QFileInfoList list = purchaseInfoDir.entryInfoList();
    QFileInfoList::const_iterator it( list.begin() );
    QFileInfo fi;

    while ( it != list.end() ) {
        fi = *it;
        returnList.append( fi.fileName() );
        ++it;
    }
     debug() << "Done parsing previous purchases!";
    return returnList;

}

void GroovesharkRedownloadHandler::redownload( GroovesharkDownloadInfo info )
{

    if ( m_albumDownloader == 0 )
    {
        m_albumDownloader = new GroovesharkAlbumDownloader();
        connect( m_albumDownloader, SIGNAL( downloadComplete( bool ) ), this, SLOT( albumDownloadComplete( bool ) ) );
    }


    if (m_downloadDialog == 0) {
        m_downloadDialog = new GroovesharkDownloadDialog(m_parent);
         connect( m_downloadDialog, SIGNAL( downloadAlbum( GroovesharkDownloadInfo ) ), m_albumDownloader, SLOT( downloadAlbum( GroovesharkDownloadInfo ) ) );
    }

    debug() << "Showing download dialog";
    m_downloadDialog->setDownloadInfo( info );
    m_downloadDialog->show();
}

void
GroovesharkRedownloadHandler::selectionDialogCancelled( )
{
    if (m_redownloadDialog != 0) {
        m_redownloadDialog->hide();
        delete m_redownloadDialog;
        m_redownloadDialog = 0;
    }
}

void
GroovesharkRedownloadHandler::albumDownloadComplete( bool success )
{
    Q_UNUSED( success );
    //cleanup time!

    if (m_downloadDialog != 0) {
       delete m_downloadDialog;
       m_downloadDialog = 0;
    }
    if (m_albumDownloader != 0) {
        delete m_albumDownloader;
        m_albumDownloader = 0;
    }

}

void
GroovesharkRedownloadHandler::fetchServerSideRedownloadList()
{

    DEBUG_BLOCK

    //do we have an email set, if not, ask the user for one.
    GroovesharkConfig config;

    QString email = config.email();

    if ( email.isEmpty() )
    {
        //TODO ask for the email
        return;
    }

    QString redownloadApiUrl = "http://grooveshark.com/buy/redownload_xml?email=" + email;

    m_redownloadApiJob = KIO::storedGet( KUrl( redownloadApiUrl ), KIO::NoReload, KIO::HideProgressInfo );
    Amarok::Components::logger()->newProgressOperation( m_redownloadApiJob, i18n( "Getting list of previous Grooveshark.com purchases" ) );
    connect( m_redownloadApiJob, SIGNAL( result( KJob* ) ), SLOT( redownloadApiResult( KJob* ) ) );

}

void GroovesharkRedownloadHandler::redownloadApiResult( KJob* job )
{

DEBUG_BLOCK

 if ( !job->error() == 0 )
    {
        //TODO: error handling here
        debug() << "Job error... " << job->error();
        return ;
    }
    if ( job != m_redownloadApiJob ) {
        debug() << "Wrong job...";
        return ; //not the right job, so let's ignore it
    }

    KIO::StoredTransferJob* const storedJob = static_cast<KIO::StoredTransferJob*>( job );
    QString resultXml = QString( storedJob->data() );

    debug() << endl << endl << "result: " << resultXml;


    QList<GroovesharkDownloadInfo> previousPurchasesInfoList;

    QDomDocument doc;
    doc.setContent( resultXml );

    QDomNodeList downloads = doc.elementsByTagName( "download" );
    for( int i = 0; i < downloads.size(); i++ )
    {
        QDomElement downloadElement = downloads.item( i ).toElement();
        GroovesharkDownloadInfo info;
        if ( info.initFromRedownloadXml( downloadElement ) )
            previousPurchasesInfoList << info;
    }


    if (m_redownloadDialog == 0) {
        m_redownloadDialog = new GroovesharkRedownloadDialog( m_parent );
        //connect( m_redownloadDialog, SIGNAL( redownload( const QString &) ), this, SLOT( redownload( const QString &) ) );
        connect( m_redownloadDialog, SIGNAL( redownload( GroovesharkDownloadInfo ) ), this, SLOT( redownload( GroovesharkDownloadInfo ) ) );
        connect( m_redownloadDialog, SIGNAL(cancelled() ), this, SLOT( selectionDialogCancelled() ) );
    }

    m_redownloadDialog->setRedownloadItems( previousPurchasesInfoList );

    m_redownloadDialog->show();

}


#include "GroovesharkRedownloadHandler.moc"
