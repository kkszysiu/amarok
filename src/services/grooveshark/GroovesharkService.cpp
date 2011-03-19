/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
 * Copyright (c) 2008 Leo Franchi <lfranchi@kde.org>                                    *
 * Copyright (c) 2009 Casey Link <unnamedrambler@gmail.com>                             *
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

#define DEBUG_PREFIX "GroovesharkService"
#include "core/support/Debug.h"

#include "GroovesharkService.h"

#include "AvatarDownloader.h"
#include "EngineController.h"
#include "biases/GroovesharkBias.h"
#include "biases/WeeklyTopBias.h"
#include "browsers/CollectionTreeItem.h"
#include "browsers/CollectionTreeItemModelBase.h"
#include "GroovesharkServiceCollection.h"
#include "GroovesharkServiceConfig.h"
#include "LoveTrackAction.h"
//#include "SimilarArtistsAction.h"
#include "GroovesharkTreeModel.h"
#include "GroovesharkTreeView.h"
//#include "ScrobblerAdapter.h"
#include "widgets/FlowLayout.h"
#include "GlobalCollectionActions.h"
#include "GlobalCurrentTrackActions.h"
#include "core-impl/collections/support/CollectionManager.h"
//#include "core/capabilities/GroovesharkCapability.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"
#include "meta/GroovesharkMeta.h"
#include "playlist/PlaylistModelStack.h"
#include "widgets/SearchWidget.h"
#include "CustomBias.h"
#include "NetworkAccessManagerProxy.h"

//#include <grooveshark/Audioscrobbler> // from libgrooveshark
//#include <grooveshark/XmlQuery>
#include <lib/QGrooveshark.h>

#include <KLocale>
#include <KPasswordDialog>
#include <KStandardDirs>
#include <solid/networking.h>

#include <QComboBox>
#include <QCryptographicHash>
#include <QGroupBox>
#include <QNetworkReply>
#include <QPainter>
#include <QImage>
#include <QFrame>
#include <QTextDocument>        //Qt::escape

AMAROK_EXPORT_SERVICE_PLUGIN( grooveshark, GroovesharkServiceFactory )

QString md5( const QByteArray& src )
{
    QByteArray const digest = QCryptographicHash::hash( src, QCryptographicHash::Md5 );
    return QString::fromLatin1( digest.toHex() ).rightJustified( 32, '0' );
}

GroovesharkServiceFactory::GroovesharkServiceFactory( QObject *parent, const QVariantList &args )
    : ServiceFactory( parent, args )
{
    KPluginInfo pluginInfo(  "amarok_service_grooveshark.desktop", "services" );
    pluginInfo.setConfig( config() );
    m_info = pluginInfo;
}

void
GroovesharkServiceFactory::init()
{
   /* Fancy network detection is nice, but buggy if you're stepping outside your currently selected
    * backend -- and since this is currently the only service using it, it makes it seem like there's just
    * a last.fm bug.  Disable until such a time as *all* the Internet services go away and are replaced by
    * helpful text describing how to change your backend if your network is actually running.
    */
   // if( Solid::Networking::status() == Solid::Networking::Unknown ) // No working solid network backend, so force creation of the service
    //{
        ServiceBase *service = createGroovesharkService();
        if( service )
        {
            m_activeServices << service;
            m_initialized = true;
            emit newService( service );
        }
   /* }
    else
    {
        if( Solid::Networking::status() == Solid::Networking::Connected )
        {
            ServiceBase *service = createGroovesharkService();
            if( service )
            {
                m_activeServices << service;
                m_initialized = true;
                emit newService( service );
            }
        }

            connect( Solid::Networking::notifier(), SIGNAL( shouldConnect() ),
                    this, SLOT( slotCreateGroovesharkService() ) );
            connect( Solid::Networking::notifier(), SIGNAL( shouldDisconnect() ),
                        this, SLOT( slotRemoveGroovesharkService() ) );
    } */
}

void
GroovesharkServiceFactory::slotCreateGroovesharkService()
{
    if( !m_initialized ) // Until we can remove a service when networking gets disabled, only create it the first time.
    {
        ServiceBase *service = createGroovesharkService();
        if( service )
        {
            m_activeServices << service;
            m_initialized = true;
            emit newService( service );
        }
    }
}

void
GroovesharkServiceFactory::slotRemoveGroovesharkService()
{
    if( m_activeServices.size() == 0 )
        return;

    m_initialized = false;
    emit removeService( m_activeServices.first() );
    m_activeServices.clear();
}

ServiceBase*
GroovesharkServiceFactory::createGroovesharkService()
{
    GroovesharkServiceConfig config;

    //  The user activated the service, but didn't fill the username/password? Don't start it.
//     if ( config.username().isEmpty() || config.password().isEmpty() )
//         return 0;

    ServiceBase* service = new GroovesharkService( this, "Grooveshark", config.username(), config.password(), config.sessionKey(), config.scrobble(), config.fetchSimilar() );
    return service;
}


QString
GroovesharkServiceFactory::name()
{
    return "Grooveshark";
}

KConfigGroup
GroovesharkServiceFactory::config()
{
    return Amarok::config( GroovesharkServiceConfig::configSectionName() );
}


GroovesharkService::GroovesharkService( GroovesharkServiceFactory* parent, const QString &name, const QString &username, QString password, const QString& sessionKey, bool scrobble, bool fetchSimilar )
    : ServiceBase( name, parent, false ),
      m_inited( false),
      m_scrobble( scrobble ),
      m_grooveshark( 0 ),
      m_collection( 0 ),
      m_polished( false ),
      m_avatarLabel( 0 ),
      m_profile( 0 ),
      m_userinfo( 0 ),
      m_userName( username ),
      m_sessionKey( sessionKey ),
      m_userNameArray( 0 ),
      m_sessionKeyArray( 0 ),
      m_groovesharkBiasFactory( 0 ),
      m_weeklyTopBiasFactory( 0 )
{
    DEBUG_BLOCK

    Q_UNUSED( sessionKey );
    Q_UNUSED( fetchSimilar ); // TODO implement..

    setShortDescription( i18n( "Grooveshark: The social music revolution" ) );
    setIcon( KIcon( "view-services-grooveshark-amarok" ) );
    setLongDescription( i18n( "Grooveshark is a popular online service that provides personal radio stations and music recommendations. A personal listening station is tailored based on your listening habits and provides you with recommendations for new music. It is also possible to play stations with music that is similar to a particular artist as well as listen to streams from people you have added as friends or that Grooveshark considers your musical \"neighbors\"" ) );
    setImagePath( KStandardDirs::locate( "data", "amarok/images/hover_info_grooveshark.png" ) );

    if( !username.isEmpty() && !password.isEmpty() )
        init();

}


GroovesharkService::~GroovesharkService()
{
    DEBUG_BLOCK

    delete m_groovesharkBiasFactory;
    delete m_weeklyTopBiasFactory;
    delete[] m_userNameArray;
    delete[] m_sessionKeyArray;

    if( m_collection && CollectionManager::instance() )
    {
        CollectionManager::instance()->removeUnmanagedCollection( m_collection );
        delete m_collection;
        m_collection = 0;
    }
    ms_service = 0;
}

void
GroovesharkService::init()
{
    GroovesharkServiceConfig config;
    const QString password = config.password();
    //const QString sessionKey = config.sessionKey();
    // set the global static Grooveshark::Ws stuff
    //grooveshark::ws::ApiKey = Amarok::groovesharkApiKey();
    //grooveshark::ws::SharedSecret = "fe0dcde9fcd14c2d1d50665b646335e9";
    // testing w/ official keys
    //Ws::SharedSecret = "73582dfc9e556d307aead069af110ab8";
    //Ws::ApiKey = "c8c7b163b11f92ef2d33ba6cd3c2c3c3";
    //m_userNameArray = qstrdup( m_userName.toLatin1().data() );
    //grooveshark::ws::Username = m_userNameArray;
    //if( grooveshark::nam() != The::networkAccessManager() )
    //    grooveshark::setNetworkAccessManager( The::networkAccessManager() );

    //debug() << "username:" << QString( QUrl::toPercentEncoding( grooveshark::ws::Username ) );

    //const QString authToken = md5( QString( "%1%2" ).arg( m_userName ).arg( md5( password.toUtf8() ) ).toUtf8() );

    // now authenticate w/ Grooveshark and get our session key if we don't have one
//     if( sessionKey.isEmpty() )
//     {
//         debug() << "got no saved session key, authenticating with Grooveshark";
//         QMap<QString, QString> query;
//         query[ "method" ] = "auth.getMobileSession";
//         query[ "username" ] = m_userName;
//         query[ "authToken" ] = authToken;
//         //m_jobs[ "auth" ] = grooveshark::ws::post( query );
// 
//         //connect( m_jobs[ "auth" ], SIGNAL( finished() ), SLOT( onAuthenticated() ) );
// 
//     } else
//     {
//         debug() << "using saved sessionkey from Grooveshark";
//         m_sessionKeyArray = qstrdup( sessionKey.toLatin1().data() );
//         //grooveshark::ws::SessionKey = m_sessionKeyArray;
//         m_sessionKey = sessionKey;
// 
//         //if( m_scrobble )
//         //    m_scrobbler = new ScrobblerAdapter( this, "ark" );
//         QMap< QString, QString > params;
//         params[ "method" ] = "user.getInfo";
//         //m_jobs[ "getUserInfo" ] = grooveshark::ws::post( params );
// 
//         //connect( m_jobs[ "getUserInfo" ], SIGNAL( finished() ), SLOT( onGetUserInfo() ) );
//     }

    //if( !m_polished )
    //    polish();
    grooveshark.startAuth();

    connect(&grooveshark, SIGNAL(tokenReceived()), SLOT(initLogging()));
    
    //We have no use for searching currently..
    m_searchWidget->setVisible( false );

    // enable custom bias
    m_groovesharkBiasFactory = new Dynamic::GroovesharkBiasFactory();
    Dynamic::CustomBias::registerNewBiasFactory( m_groovesharkBiasFactory );

    m_weeklyTopBiasFactory = new Dynamic::WeeklyTopBiasFactory();
    Dynamic::CustomBias::registerNewBiasFactory( m_weeklyTopBiasFactory );

    m_collection = new Collections::GroovesharkServiceCollection( m_userName );
    CollectionManager::instance()->addUnmanagedCollection( m_collection, CollectionManager::CollectionDisabled );


    //add the "play similar artists" action to all artist
    //The::globalCollectionActions()->addArtistAction( new SimilarArtistsAction( this ) );
    The::globalCollectionActions()->addTrackAction( new LoveTrackAction( this ) );


    QAction * loveAction = new QAction( KIcon( "love-amarok" ), i18n( "Grooveshark: Love" ), this );
    connect( loveAction, SIGNAL( triggered() ), this, SLOT( love() ) );
    loveAction->setShortcut( i18n( "Ctrl+L" ) );
    The::globalCurrentTrackActions()->addAction( loveAction );


    Q_ASSERT( ms_service == 0 );
    ms_service = this;
    m_serviceready = true;

    m_inited = true;
}


void
GroovesharkService::initLogging()
{
    GroovesharkServiceConfig config;
    const QString password = config.password();
    
    grooveshark.authoriseUser(m_userName, password);
    connect(&grooveshark, SIGNAL(authenticationSucceed()), SLOT(userAuthenticated()));
}

void
GroovesharkService::userAuthenticated()
{
    GroovesharkServiceConfig config;
    /*
    if( lfm.children( "error" ).size() > 0 )
    {
        debug() << "error from authenticating with Grooveshark service:" << lfm.text();
        config.clearSessionKey();
        break;
    }
    m_sessionKey = lfm[ "session" ][ "key" ].text();
    */

//             m_sessionKeyArray = qstrdup( m_sessionKey.toLatin1().data() );
//             //grooveshark::ws::SessionKey = m_sessionKeyArray;
//             config.setSessionKey( m_sessionKey );
//             config.save();
// 
//             //if( m_scrobble )
//             //    m_scrobbler = new ScrobblerAdapter( this, "ark" );
//             QMap< QString, QString > params;
//             params[ "method" ] = "user.getInfo";
//             //m_jobs[ "getUserInfo" ] = grooveshark::ws::post( params );
// 
//             //connect( m_jobs[ "getUserInfo" ], SIGNAL( finished() ), SLOT( onGetUserInfo() ) );
// 
//             break;
//         }
//         case QNetworkReply::AuthenticationRequiredError:
//             Amarok::Components::logger()->longMessage( i18nc("Grooveshark: errorMessage", "Either the username was not recognized, or the password was incorrect." ) );
//             break;
// 
//         default:
//             Amarok::Components::logger()->longMessage( i18nc("Grooveshark: errorMessage", "There was a problem communicating with the Grooveshark services. Please try again later." ) );
//             break;
//     }
//     m_jobs[ "auth" ]->deleteLater();
}

void
GroovesharkService::onGetUserInfo()
{
    DEBUG_BLOCK
    if( !m_jobs[ "getUserInfo" ] )
    {
        debug() << "GOT RESULT FROM USER QUERY, but no object..!";
        return;
    }
    switch (m_jobs[ "getUserInfo" ]->error())
    {
        case QNetworkReply::NoError:
        {
            debug() << "GroovesharkService::onGetUserInfo() getUserInfo";
            /*
            try
            {
                grooveshark::XmlQuery lfm( m_jobs[ "getUserInfo" ]->readAll() );

                m_country = lfm["user"]["country"].text();
                m_age = lfm["user"]["age"].text();
                m_gender = lfm["user"]["gender"].text();
                m_playcount = lfm["user"]["playcount"].text();
                m_subscriber = lfm["user"]["subscriber"].text() == "1";

                debug() << "profile info "  << m_country << " " << m_age << " " << m_gender << " " << m_playcount << " " << m_subscriber;
                if( !lfm["user"][ "image" ].text().isEmpty() )
                {
                    debug() << "profile avatar: " <<lfm["user"][ "image" ].text();
                    AvatarDownloader* downloader = new AvatarDownloader();
                    KUrl url( lfm["user"][ "image" ].text() );
                    downloader->downloadAvatar( m_userName,  url);
                    connect( downloader, SIGNAL(avatarDownloaded(const QString&, QPixmap)),
                                         SLOT(onAvatarDownloaded(const QString&, QPixmap)) );
                }
                updateProfileInfo();

            } catch( grooveshark::ws::ParseError& e )
            {
                debug() << "Got exception in parsing from Grooveshark:" << e.what();
            }
            */
            break;
        } case QNetworkReply::AuthenticationRequiredError:
        debug() << "Grooveshark: errorMessage: Sorry, we don't recognise that username, or you typed the password incorrectly.";
            break;

        default:
            debug() << "Grooveshark: errorMessage: There was a problem communicating with the Grooveshark services. Please try again later.";
            break;
    }

    m_jobs[ "getUserInfo" ]->deleteLater();
}

void
GroovesharkService::onAvatarDownloaded( const QString &username, QPixmap avatar )
{
    DEBUG_BLOCK
    if( username == m_userName && !avatar.isNull() ) {

        if( !m_polished )
            polish();

        GroovesharkTreeModel* lfm = dynamic_cast<GroovesharkTreeModel*>( model() );

        int m = lfm->avatarSize();
        avatar = avatar.scaled( m, m, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        lfm->prepareAvatar( avatar, m );
        m_avatar = avatar;

        if( m_avatarLabel )
            m_avatarLabel->setPixmap( m_avatar );
    }
    sender()->deleteLater();
}

void
GroovesharkService::updateEditHint( int index )
{
    if( !m_customStationEdit )
        return;
    QString hint;
    switch ( index ) {
        case 0:
            hint = i18n( "Enter an artist name" );
            break;
        case 1:
            hint = i18n( "Enter a tag" );
            break;
        case 2:
            hint = i18n( "Enter a Grooveshark user name" );
            break;
        default:
            return;
    }
    m_customStationEdit->setClickMessage( hint );
}

void
GroovesharkService::updateProfileInfo()
{
    if( m_userinfo )
    {
        m_userinfo->setText( i18n( "Username: ") + Qt::escape( m_userName ) );
    }

    if( m_profile && !m_playcount.isEmpty() )
    {
        m_profile->setText( i18np( "Play Count: %1 play", "Play Count: %1 plays", m_playcount.toInt() ) );
    }
}

void
GroovesharkService::polish()
{
    if( !m_inited )
    {
        KPasswordDialog dlg( 0 , KPasswordDialog::ShowUsernameLine );
        dlg.setPrompt( i18n( "Enter login information for Grooveshark" ) );
        if( !dlg.exec() )
            return; //the user canceled

            m_userName = dlg.username();
        const QString password = dlg.password();
        if( password.isEmpty() || m_userName.isEmpty() )
            return; // We can't create the service if we don't get the details..
        GroovesharkServiceConfig config;
        config.setPassword( password );
        config.setUsername( m_userName );
        config.save();
        init();
    }
    if( !m_polished )
    {
        GroovesharkTreeView* view = new GroovesharkTreeView( this );
        view->setFrameShape( QFrame::NoFrame );
        view->setDragEnabled ( true );
        view->setSortingEnabled( false );
        view->setDragDropMode ( QAbstractItemView::DragOnly );
        setView( view );
        setModel( new GroovesharkTreeModel( m_userName, this ) );

        //m_bottomPanel->setMaximumHeight( 300 );
        m_bottomPanel->hide();

        m_topPanel->setMaximumHeight( 300 );
        KHBox * outerProfilebox = new KHBox( m_topPanel );
        outerProfilebox->setSpacing(1);
        outerProfilebox->setMargin(0);

        m_avatarLabel = new QLabel(outerProfilebox);
        if( !m_avatar )
        {
            int m = dynamic_cast<GroovesharkTreeModel*>( model() )->avatarSize();
            m_avatarLabel->setPixmap( KIcon( "filename-artist-amarok" ).pixmap(m, m) );
            m_avatarLabel->setFixedSize( m, m );
        }
        else
        {
            m_avatarLabel->setPixmap( m_avatar );
            m_avatarLabel->setFixedSize( m_avatar.width(), m_avatar.height() );
            m_avatarLabel->setMargin( 5 );
        }

        KVBox * innerProfilebox = new KVBox( outerProfilebox );
        innerProfilebox->setSpacing(0);
        innerProfilebox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        m_userinfo = new QLabel(innerProfilebox);
        m_userinfo->setText( m_userName );
        m_profile = new QLabel(innerProfilebox);
        m_profile->setText(QString());
        updateProfileInfo();


        QGroupBox *customStation = new QGroupBox( i18n( "Create a Custom Grooveshark Station" ), m_topPanel );
        m_customStationCombo = new QComboBox;
        QStringList choices;
        choices << i18n( "Artist" ) << i18n( "Tag" ) << i18n( "User" );
        m_customStationCombo->insertItems(0, choices);
        m_customStationEdit = new KLineEdit;
        m_customStationEdit->setClearButtonShown( true );
        updateEditHint( m_customStationCombo->currentIndex() );
        m_customStationButton = new QPushButton;
        m_customStationButton->setObjectName( "customButton" );
        m_customStationButton->setIcon( KIcon( "media-playback-start-amarok" ) );
        QHBoxLayout *hbox = new QHBoxLayout();
        hbox->addWidget(m_customStationCombo);
        hbox->addWidget(m_customStationEdit);
        hbox->addWidget(m_customStationButton);
        customStation->setLayout(hbox);

        connect( m_customStationEdit, SIGNAL( returnPressed() ), this, SLOT( playCustomStation() ) );
        connect( m_customStationButton, SIGNAL( clicked() ), this, SLOT( playCustomStation() ) );
        connect( m_customStationCombo, SIGNAL( currentIndexChanged(int) ), this, SLOT( updateEditHint(int) ));

        QList<int> levels;
        levels << CategoryId::Genre << CategoryId::Album;
        m_polished = true;
    }
}


void
GroovesharkService::love()
{
    DEBUG_BLOCK

    Meta::TrackPtr track = The::engineController()->currentTrack();
    Grooveshark::Track* groovesharkTrack = dynamic_cast< Grooveshark::Track* >( track.data() );

    if( groovesharkTrack )
    {
        groovesharkTrack->love();
        Amarok::Components::logger()->shortMessage( i18nc( "As in, grooveshark", "Loved Track: %1", track->prettyName() ) );
    }
    else
    {
        //m_scrobbler->loveTrack( track );
    }

}

void GroovesharkService::love( Meta::TrackPtr track )
{
    DEBUG_BLOCK
    //m_scrobbler->loveTrack( track );
}


void
GroovesharkService::ban()
{
    DEBUG_BLOCK

    Meta::TrackPtr track = The::engineController()->currentTrack();
    Grooveshark::Track* groovesharkTrack = dynamic_cast< Grooveshark::Track* >( track.data() );
    if( groovesharkTrack )
        groovesharkTrack->ban();
}


void
GroovesharkService::skip()
{
    DEBUG_BLOCK

    Meta::TrackPtr track = The::engineController()->currentTrack();
    Grooveshark::Track* groovesharkTrack = dynamic_cast< Grooveshark::Track* >( track.data() );
    if( groovesharkTrack )
        groovesharkTrack->skip();
}


GroovesharkService *GroovesharkService::ms_service = 0;


namespace The
{
    GroovesharkService *groovesharkService()
    {
        return GroovesharkService::ms_service;
    }
}

void GroovesharkService::playCustomStation()
{
    // 0 - everything
    // 1 - artists
    // 2 - albums
    // 3 - playlists
    // 4 - users
    DEBUG_BLOCK
    QString text = m_customStationEdit->text();
    QString station;
    debug() << "Selected combo " <<m_customStationCombo->currentIndex();
    switch ( m_customStationCombo->currentIndex() ) {
        case 0:
            station = "grooveshark://everything/" + text;
            break;
        case 1:
            station = "grooveshark://artists/" + text;
            break;
        case 2:
            station = "grooveshark://albums/" + text;
            break;
        case 3:
            station = "grooveshark://playlists/" + text;
            break;
        case 4:
            station = "grooveshark://users/" + text;
            break;
        default:
            return;
    }

    if ( !station.isEmpty() ) {
        playGroovesharkStation( station );
    }
}

void GroovesharkService::playGroovesharkStation( const KUrl &url )
{
    debug() << "Meta::TrackPtr";
    Meta::TrackPtr track = CollectionManager::instance()->trackForUrl( url );
    debug() << "The::playlistController()";
    The::playlistController()->insertOptioned( track, Playlist::AppendAndPlay );
}

Collections::Collection * GroovesharkService::collection()
{
    return m_collection;
}
