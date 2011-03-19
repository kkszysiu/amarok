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

#include "GroovesharkStore.h"

#include "core/support/Amarok.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"
#include "amarokurls/AmarokUrlHandler.h"
#include "browsers/CollectionTreeItem.h"
#include "browsers/SingleCollectionTreeItemModel.h"
#include "EngineController.h"
#include "GroovesharkConfig.h"
#include "GroovesharkDatabaseWorker.h"
#include "GroovesharkInfoParser.h"
#include "browsers/InfoProxy.h"
#include "GroovesharkUrlRunner.h"

#include "ui_GroovesharkSignupDialogBase.h"

#include "../ServiceSqlRegistry.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "core/support/Debug.h"
#include "playlist/PlaylistModelStack.h"
#include "widgets/SearchWidget.h"

#include <KAction>
#include <KMenuBar>
#include <KStandardDirs>  //locate()
#include <KTemporaryFile>
#include <KUrl>
#include <threadweaver/ThreadWeaver.h>

#include <QDateTime>
#include <QMenu>
#include <QToolButton>

#include <typeinfo>

AMAROK_EXPORT_SERVICE_PLUGIN( groovesharkstore, GroovesharkServiceFactory )

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class GroovesharkServiceFactory
////////////////////////////////////////////////////////////////////////////////////////////////////////////

GroovesharkServiceFactory::GroovesharkServiceFactory( QObject *parent, const QVariantList &args )
    : ServiceFactory( parent, args )
{
    KPluginInfo pluginInfo( "amarok_service_groovesharkstore.desktop", "services" );
    pluginInfo.setConfig( config() );
    m_info = pluginInfo;
}

void GroovesharkServiceFactory::init()
{
    DEBUG_BLOCK
    GroovesharkStore* service = new GroovesharkStore( this, "Grooveshark.com" );
    m_activeServices << service;
    m_initialized = true;
    emit newService( service );
}

QString GroovesharkServiceFactory::name()
{
    return "Grooveshark.com";
}

KConfigGroup GroovesharkServiceFactory::config()
{
    return Amarok::config( "Service_Grooveshark" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class GroovesharkStore
////////////////////////////////////////////////////////////////////////////////////////////////////////////

GroovesharkStore::GroovesharkStore( GroovesharkServiceFactory* parent, const char *name )
        : ServiceBase( name, parent )
        , m_downloadHandler( 0 )
        , m_redownloadHandler( 0 )
        , m_downloadInProgress( 0 )
        , m_currentAlbum( 0 )
        , m_streamType( GroovesharkMetaFactory::OGG )
        , m_groovesharkTimestamp( 0 )
        , m_registry( 0 )
        , m_signupInfoWidget( 0 )
{
    setObjectName(name);
    DEBUG_BLOCK
    //initTopPanel( );

    setShortDescription( i18n( "\"Fair trade\" online music store." ) );
    setIcon( KIcon( "view-services-grooveshark-amarok" ) );

    // xgettext: no-c-format
    setLongDescription( i18n( "Grooveshark.com is a different kind of record company with the motto \"We are not evil!\" 50% of every purchase goes directly to the artist and if you purchase an album through Amarok, the Amarok project receives a 10% commission. Grooveshark.com also offers \"all you can eat\" memberships that lets you download as much of their music as you like." ) );
    setImagePath( KStandardDirs::locate( "data", "amarok/images/hover_info_grooveshark.png" ) );


    //initBottomPanel();
//    m_currentlySelectedItem = 0;

    m_polished = false;
    //polish( );  //FIXME not happening when shown for some reason


    //do this stuff now to make us function properly as a track provider on startup. The expensive stuff will
    //not happen untill the model is added to the view anyway.
    GroovesharkMetaFactory * metaFactory = new GroovesharkMetaFactory( "grooveshark", this );
    
    GroovesharkConfig config;
    if ( config.isMember() ) {
        setMembership( config.membershipType(), config.username(), config.password() );
        metaFactory->setMembershipInfo( config.membershipPrefix(), m_username, m_password );
    }

    setStreamType( config.streamType() );

    metaFactory->setStreamType( m_streamType );
    m_registry = new ServiceSqlRegistry( metaFactory );
    m_collection = new Collections::GroovesharkSqlCollection( "grooveshark", "Grooveshark.com", metaFactory, m_registry );
    m_serviceready = true;
    CollectionManager::instance()->addUnmanagedCollection( m_collection, CollectionManager::CollectionDisabled );
    emit( ready() );
}

GroovesharkStore::~GroovesharkStore()
{
    CollectionManager::instance()->removeUnmanagedCollection( m_collection );
    delete m_registry;
    delete m_collection;
}


void GroovesharkStore::download( )
{
    DEBUG_BLOCK
    if ( m_downloadInProgress )
        return;

    if ( !m_polished )
        polish();

    debug() << "here";

    //check if we need to start a download or show the signup dialog
    if( !m_isMember || m_membershipType != GroovesharkConfig::DOWNLOAD )
    {
        showSignupDialog();
        return;
    }

    m_downloadInProgress = true;
    m_downloadAlbumButton->setEnabled( false );

    if ( !m_downloadHandler )
    {
        m_downloadHandler = new GroovesharkDownloadHandler();
        m_downloadHandler->setParent( this );
        connect( m_downloadHandler, SIGNAL( downloadCompleted( bool ) ), this, SLOT( downloadCompleted( bool ) ) );
    }

    if ( m_currentAlbum != 0 )
        m_downloadHandler->downloadAlbum( m_currentAlbum );
}


void GroovesharkStore::download( Meta::GroovesharkTrack * track )
{
    Meta::GroovesharkAlbum * album = dynamic_cast<Meta::GroovesharkAlbum *>( track->album().data() );
    if ( album )
        download( album );
}

void GroovesharkStore::download( Meta::GroovesharkAlbum * album )
{

    DEBUG_BLOCK
    if ( m_downloadInProgress )
        return;

    if ( !m_polished )
        polish();

    m_downloadInProgress = true;
    m_downloadAlbumButton->setEnabled( false );

    if ( !m_downloadHandler )
    {
        m_downloadHandler = new GroovesharkDownloadHandler();
        m_downloadHandler->setParent( this );
        connect( m_downloadHandler, SIGNAL( downloadCompleted( bool ) ), this, SLOT( downloadCompleted( bool ) ) );
    }

    m_downloadHandler->downloadAlbum( album );
}


void GroovesharkStore::initTopPanel( )
{

    QMenu *filterMenu = new QMenu( 0 );

    QAction *action = filterMenu->addAction( i18n("Artist") );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( sortByArtist() ) );

    action = filterMenu->addAction( i18n( "Artist / Album" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( sortByArtistAlbum() ) );

    action = filterMenu->addAction( i18n( "Album" ) ) ;
    connect( action, SIGNAL( triggered( bool ) ), SLOT( sortByAlbum() ) );

    action = filterMenu->addAction( i18n( "Genre / Artist" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( sortByGenreArtist() ) );

    action = filterMenu->addAction( i18n( "Genre / Artist / Album" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( sortByGenreArtistAlbum() ) );

    KAction *filterMenuAction = new KAction( KIcon( "preferences-other" ), i18n( "Sort Options" ), this );
    filterMenuAction->setMenu( filterMenu );

    m_searchWidget->toolBar()->addSeparator();
    m_searchWidget->toolBar()->addAction( filterMenuAction );

    QToolButton *tbutton = qobject_cast<QToolButton*>( m_searchWidget->toolBar()->widgetForAction( filterMenuAction ) );
    if( tbutton )
        tbutton->setPopupMode( QToolButton::InstantPopup );

    QMenu * actionsMenu = new QMenu( 0 );

    action = actionsMenu->addAction( i18n( "Re-download" ) );
    connect( action, SIGNAL( triggered( bool) ), SLOT( processRedownload() ) );

    m_updateAction = actionsMenu->addAction( i18n( "Update Database" ) );
    connect( m_updateAction, SIGNAL( triggered( bool) ), SLOT( updateButtonClicked() ) );

    KAction *actionsMenuAction = new KAction( KIcon( "list-add" ), i18n( "Tools" ), this );
    actionsMenuAction->setMenu( actionsMenu );

    m_searchWidget->toolBar()->addAction( actionsMenuAction );

    tbutton = qobject_cast<QToolButton*>( m_searchWidget->toolBar()->widgetForAction( actionsMenuAction ) );
    if( tbutton )
        tbutton->setPopupMode( QToolButton::InstantPopup );

}

void GroovesharkStore::initBottomPanel()
{
    //m_bottomPanel->setMaximumHeight( 24 );

    m_downloadAlbumButton = new QPushButton;
    m_downloadAlbumButton->setParent( m_bottomPanel );

    GroovesharkConfig config;
    if ( config.isMember() && config.membershipType() == GroovesharkConfig::DOWNLOAD )
    {
        m_downloadAlbumButton->setText( i18n( "Download Album" ) );
        m_downloadAlbumButton->setEnabled( false );
    }
    else if ( config.isMember() )
        m_downloadAlbumButton->hide();
    else
    {
        m_downloadAlbumButton->setText( i18n( "Signup" ) );
        m_downloadAlbumButton->setEnabled( true );
    }

    m_downloadAlbumButton->setObjectName( "downloadButton" );
    m_downloadAlbumButton->setIcon( KIcon( "download-amarok" ) );
    

    connect( m_downloadAlbumButton, SIGNAL( clicked() ) , this, SLOT( download() ) );
}


void GroovesharkStore::updateButtonClicked()
{
    DEBUG_BLOCK
    m_updateAction->setEnabled( false );
    updateGroovesharkList();
}


bool GroovesharkStore::updateGroovesharkList()
{
    DEBUG_BLOCK
    //download new list from grooveshark

     debug() << "GroovesharkStore: start downloading xml file";


    KTemporaryFile tempFile;
    tempFile.setSuffix( ".bz2" );
    tempFile.setAutoRemove( false );  //file must be removed later
    if( !tempFile.open() )
    {
        return false; //error
    }

    m_tempFileName = tempFile.fileName();

    m_listDownloadJob = KIO::file_copy( KUrl( "http://grooveshark.com/info/album_info_xml.bz2" ),  KUrl( m_tempFileName ), 0700 , KIO::HideProgressInfo | KIO::Overwrite );
    Amarok::Components::logger()->newProgressOperation( m_listDownloadJob, i18n( "Downloading Grooveshark.com Database" ), this, SLOT( listDownloadCancelled() ) );

    connect( m_listDownloadJob, SIGNAL( result( KJob * ) ),
            this, SLOT( listDownloadComplete( KJob * ) ) );


    return true;
}


void GroovesharkStore::listDownloadComplete( KJob * downLoadJob )
{
   DEBUG_BLOCK
   debug() << "GroovesharkStore: xml file download complete";

    if ( downLoadJob != m_listDownloadJob ) {
        debug() << "wrong job, ignoring....";
        return ; //not the right job, so let's ignore it
    }

    m_updateAction->setEnabled( true );
    if ( !downLoadJob->error() == 0 )
    {
        debug() << "Got an error, bailing out: " << downLoadJob->errorString();
        //TODO: error handling here
        return ;
    }


    Amarok::Components::logger()->shortMessage( i18n( "Updating the local Grooveshark database."  ) );
    GroovesharkXmlParser * parser = new GroovesharkXmlParser( m_tempFileName );
    parser->setDbHandler( new GroovesharkDatabaseHandler() );
    connect( parser, SIGNAL( doneParsing() ), SLOT( doneParsing() ) );

    ThreadWeaver::Weaver::instance()->enqueue( parser );
}


void GroovesharkStore::listDownloadCancelled( )
{
    DEBUG_BLOCK

    m_listDownloadJob->kill();
    m_listDownloadJob = 0;
    debug() << "Aborted xml download";

    m_updateAction->setEnabled( true );
}



void GroovesharkStore::doneParsing()
{
    debug() << "GroovesharkStore: done parsing";
    m_collection->emitUpdated();

    //update the last update timestamp

    GroovesharkConfig config;
    if ( m_groovesharkTimestamp == 0 )
        config.setLastUpdateTimestamp( QDateTime::currentDateTime().toTime_t() );
    else
        config.setLastUpdateTimestamp( m_groovesharkTimestamp );

    config.save();
}


void GroovesharkStore::processRedownload( )
{
    debug() << "Process redownload";

    if ( m_redownloadHandler == 0 )
    {
        m_redownloadHandler = new GroovesharkRedownloadHandler( this );
    }
    m_redownloadHandler->showRedownloadDialog();
}


void GroovesharkStore::downloadCompleted( bool )
{
    delete m_downloadHandler;
    m_downloadHandler = 0;

    m_downloadAlbumButton->setEnabled( true );
    m_downloadInProgress = false;

    debug() << "Purchase operation complete";

    //TODO: display some kind of success dialog here?
}


void GroovesharkStore::itemSelected( CollectionTreeItem * selectedItem )
{
    DEBUG_BLOCK

    //only care if the user has a download membership
    if( !m_isMember || m_membershipType != GroovesharkConfig::DOWNLOAD )
        return;

    //we only enable the purchase button if there is only one item selected and it happens to
    //be an album or a track
    Meta::DataPtr dataPtr = selectedItem->data();

    if ( typeid( * dataPtr.data() ) == typeid( Meta::GroovesharkTrack ) )  {

        debug() << "is right type (track)";
        Meta::GroovesharkTrack * track = static_cast<Meta::GroovesharkTrack *> ( dataPtr.data() );
        m_currentAlbum = static_cast<Meta::GroovesharkAlbum *> ( track->album().data() );
        m_downloadAlbumButton->setEnabled( true );

    } else if ( typeid( * dataPtr.data() ) == typeid( Meta::GroovesharkAlbum ) ) {

        m_currentAlbum = static_cast<Meta::GroovesharkAlbum *> ( dataPtr.data() );
        debug() << "is right type (album) named " << m_currentAlbum->name();

        m_downloadAlbumButton->setEnabled( true );
    } else {

        debug() << "is wrong type";
        m_downloadAlbumButton->setEnabled( false );

    }
}


void GroovesharkStore::addMoodyTracksToPlaylist( const QString &mood, int count )
{
    GroovesharkDatabaseWorker * databaseWorker = new GroovesharkDatabaseWorker();
    databaseWorker->fetchTrackswithMood( mood, count, m_registry );
    connect( databaseWorker, SIGNAL( gotMoodyTracks( Meta::TrackList ) ), this, SLOT( moodyTracksReady(Meta::TrackList ) ) );

    ThreadWeaver::Weaver::instance()->enqueue( databaseWorker );
}


void GroovesharkStore::polish()
{
    DEBUG_BLOCK;

    if (!m_polished) {
        m_polished = true;

        initTopPanel( );
        initBottomPanel();

        QList<int> levels;
        levels << CategoryId::Genre << CategoryId::Artist << CategoryId::Album;

        m_groovesharkInfoParser = new GroovesharkInfoParser();

        setInfoParser( m_groovesharkInfoParser );
        setModel( new SingleCollectionTreeItemModel( m_collection, levels ) );

        connect( m_contentView, SIGNAL( itemSelected( CollectionTreeItem * ) ), this, SLOT( itemSelected( CollectionTreeItem * ) ) );

        //add a custom url runner

        GroovesharkUrlRunner * runner = new GroovesharkUrlRunner();

        connect( runner, SIGNAL( showFavorites() ), this, SLOT( showFavoritesPage() ) );
        connect( runner, SIGNAL( showHome() ), this, SLOT( showHomePage() ) );
        connect( runner, SIGNAL( showRecommendations() ), this, SLOT( showRecommendationsPage() ) );
        connect( runner, SIGNAL( buyOrDownload( const QString & ) ), this, SLOT( download( const QString & ) ) );
        connect( runner, SIGNAL( removeFromFavorites( const QString & ) ), this, SLOT( removeFromFavorites( const QString & ) ) );

        The::amarokUrlHandler()->registerRunner( runner, runner->command() );
    }

    const KUrl url( KStandardDirs::locate( "data", "amarok/data/" ) );
    QString imagePath = url.url();

    GroovesharkInfoParser * parser = dynamic_cast<GroovesharkInfoParser *> ( infoParser() );
    if ( parser )
        parser->getFrontPage();

    //get a mood map we can show to the cloud view

    GroovesharkDatabaseWorker * databaseWorker = new GroovesharkDatabaseWorker();
    databaseWorker->fetchMoodMap();
    connect( databaseWorker, SIGNAL( gotMoodMap(QMap< QString, int >) ), this, SLOT( moodMapReady(QMap< QString, int >) ) );
    ThreadWeaver::Weaver::instance()->enqueue( databaseWorker );

    checkForUpdates();
}



void GroovesharkStore::setMembership( int type, const QString & username, const QString & password)
{
    m_isMember = true;
    m_membershipType = type;
    m_username = username;
    m_password = password;
}


void GroovesharkStore::moodMapReady(QMap< QString, int > map)
{
    QVariantMap variantMap;
    QList<QVariant> strings;
    QList<QVariant> weights;
    QVariantMap dbusActions;

    foreach( const QString &key, map.keys() ) {

        strings << key;
        weights << map.value( key );

        QString escapedKey = key;
        escapedKey.replace( ' ', "%20" );
        QVariantMap action;
        action["component"]  = "/ServicePluginManager";
        action["function"] = "sendMessage";
        action["arg1"] = QString( "Grooveshark.com").arg( escapedKey );
        action["arg2"] = QString( "addMoodyTracks %1 10").arg( escapedKey );

        dbusActions[key] = action;

    }

    variantMap["cloud_name"] = QVariant( "Grooveshark Moods" );
    variantMap["cloud_strings"] = QVariant( strings );
    variantMap["cloud_weights"] = QVariant( weights );
    variantMap["cloud_actions"] = QVariant( dbusActions );

    The::infoProxy()->setCloud( variantMap );
}


void GroovesharkStore::setStreamType( int type )
{
    m_streamType = type;
}


void GroovesharkStore::downloadCurrentTrackAlbum()
{
    //get current track
    Meta::TrackPtr track = The::engineController()->currentTrack();

    //check if this is indeed a grooveshark track
    Capabilities::SourceInfoCapability *sic = track->create<Capabilities::SourceInfoCapability>();
    if( sic )
    {
        //is the source defined
        QString source = sic->sourceName();
        if ( source != "Grooveshark.com" ) {
            //not a Grooveshark track, so don't bother...
            delete sic;
            return;
        }
        delete sic;
    } else {
        //not a Grooveshark track, so don't bother...
        return;
    }

    //so far so good...
    //now the casting begins:

    Meta::GroovesharkTrack * groovesharkTrack = dynamic_cast<Meta::GroovesharkTrack *> ( track.data() );
    if ( !groovesharkTrack )
        return;

    Meta::GroovesharkAlbum * groovesharkAlbum = dynamic_cast<Meta::GroovesharkAlbum *> ( groovesharkTrack->album().data() );
    if ( !groovesharkAlbum )
        return;

    if ( !m_downloadHandler )
    {
        m_downloadHandler = new GroovesharkDownloadHandler();
        m_downloadHandler->setParent( this );
        connect( m_downloadHandler, SIGNAL( downloadCompleted( bool ) ), this, SLOT( downloadCompleted( bool ) ) );
    }

    m_downloadHandler->downloadAlbum( groovesharkAlbum );
}


void GroovesharkStore::checkForUpdates()
{
    m_updateTimestampDownloadJob = KIO::storedGet( KUrl( "http://grooveshark.com/info/last_update_timestamp" ), KIO::Reload, KIO::HideProgressInfo );
    connect( m_updateTimestampDownloadJob, SIGNAL( result( KJob * ) ), SLOT( timestampDownloadComplete( KJob *  ) ) );
}


void GroovesharkStore::timestampDownloadComplete( KJob *  job )
{
    DEBUG_BLOCK

    if ( !job->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( job != m_updateTimestampDownloadJob )
        return ; //not the right job, so let's ignore it


    QString timestampString = ( ( KIO::StoredTransferJob* ) job )->data();
    debug() << "Grooveshark timestamp: " << timestampString;

    bool ok;
    qulonglong groovesharkTimestamp = timestampString.toULongLong( &ok );

    GroovesharkConfig config;
    qulonglong localTimestamp = config.lastUpdateTimestamp();

    debug() << "Last update timestamp: " << QString::number( localTimestamp );

    if ( ok && groovesharkTimestamp > localTimestamp ) {
        m_groovesharkTimestamp = groovesharkTimestamp;
        updateButtonClicked();
    }
}


void GroovesharkStore::moodyTracksReady( Meta::TrackList tracks )
{
    DEBUG_BLOCK
    The::playlistController()->insertOptioned( tracks, Playlist::Replace );
}


QString GroovesharkStore::messages()
{
    QString text = i18n( "The Grooveshark.com service accepts the following messages: \n\n\taddMoodyTracks mood count: Adds a number of random tracks with the specified mood to the playlist. The mood argument must have spaces escaped with %%20" );

    return text;
}


QString GroovesharkStore::sendMessage( const QString & message )
{
    QStringList args = message.split( ' ', QString::SkipEmptyParts );

    if ( args.size() < 1 ) {
        return i18n( "ERROR: No arguments supplied" );
    }

    if ( args[0] == "addMoodyTracks" ) {
        if ( args.size() != 3 ) {
            return i18n( "ERROR: Wrong number of arguments for addMoodyTracks" );
        }

        QString mood = args[1];
        mood = mood.replace( "%20", " " );

        bool ok;
        int count = args[2].toInt( &ok );

        if ( !ok )
            return i18n( "ERROR: Parse error for argument 2 ( count )" );

        addMoodyTracksToPlaylist( mood, count );

        return i18n( "ok" );
    }

    return i18n( "ERROR: Unknown argument." );
}

void GroovesharkStore::showFavoritesPage()
{
    DEBUG_BLOCK
    m_groovesharkInfoParser->getFavoritesPage();
}

void GroovesharkStore::showHomePage()
{
    DEBUG_BLOCK
    m_groovesharkInfoParser->getFrontPage();
}

void GroovesharkStore::showRecommendationsPage()
{
    DEBUG_BLOCK
    m_groovesharkInfoParser->getRecommendationsPage();
}

void GroovesharkStore::download( const QString &sku )
{
    DEBUG_BLOCK
    debug() << "sku: " << sku;
    GroovesharkDatabaseWorker * databaseWorker = new GroovesharkDatabaseWorker();
    databaseWorker->fetchAlbumBySku( sku, m_registry );
    connect( databaseWorker, SIGNAL( gotAlbumBySku( Meta::GroovesharkAlbum * ) ), this, SLOT( download( Meta::GroovesharkAlbum * ) ) );

    ThreadWeaver::Weaver::instance()->enqueue( databaseWorker );
}

void GroovesharkStore::addToFavorites( const QString &sku )
{
    DEBUG_BLOCK
    GroovesharkConfig config;

    if( !config.isMember() )
        return;

    QString url = "http://%1:%2@%3.grooveshark.com/member/favorites?action=add_api&sku=%4";
    url = url.arg( config.username(), config.password(), config.membershipPrefix(), sku );

    debug() << "favorites url: " << url;

    m_favoritesJob = KIO::storedGet( KUrl( url ), KIO::Reload, KIO::HideProgressInfo );
    connect( m_favoritesJob, SIGNAL( result( KJob * ) ), SLOT( favoritesResult( KJob *  ) ) );
}

void GroovesharkStore::removeFromFavorites( const QString &sku )
{
    DEBUG_BLOCK
    GroovesharkConfig config;

    if( !config.isMember() )
        return;

    QString url = "http://%1:%2@%3.grooveshark.com/member/favorites?action=remove_api&sku=%4";
    url = url.arg( config.username(), config.password(), config.membershipPrefix(), sku );

    debug() << "favorites url: " << url;

    m_favoritesJob = KIO::storedGet( KUrl( url ), KIO::Reload, KIO::HideProgressInfo );
    connect( m_favoritesJob, SIGNAL( result( KJob * ) ), SLOT( favoritesResult( KJob *  ) ) );
}

void GroovesharkStore::favoritesResult( KJob* addToFavoritesJob )
{
    if( addToFavoritesJob != m_favoritesJob )
        return;

    QString result = m_favoritesJob->data();

    Amarok::Components::logger()->longMessage( result );

    //show the favorites page
    showFavoritesPage();
}

void
GroovesharkStore::showSignupDialog()
{

    if ( m_signupInfoWidget== 0 )
    {
        m_signupInfoWidget = new QDialog;
        Ui::SignupDialog ui;
        ui.setupUi( m_signupInfoWidget );
    }

     m_signupInfoWidget->show();
}



#include "GroovesharkStore.moc"


