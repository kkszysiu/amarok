/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
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

#define DEBUG_PREFIX "GroovesharkServiceSettings"

#include "GroovesharkServiceSettings.h"

#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "NetworkAccessManagerProxy.h"
#include "ui_GroovesharkConfigWidget.h"

// #include <grooveshark/Audioscrobbler> // from libgrooveshark
// #include <grooveshark/ws.h>
// #include <grooveshark/XmlQuery>

#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVBoxLayout>
#include <QRegExpValidator>

#include <KMessageBox>
#include <KPluginFactory>


K_PLUGIN_FACTORY( GroovesharkServiceSettingsFactory, registerPlugin<GroovesharkServiceSettings>(); )
K_EXPORT_PLUGIN( GroovesharkServiceSettingsFactory( "kcm_amarok_grooveshark" ) )

QString md5( const QByteArray& src )
{
    QByteArray const digest = QCryptographicHash::hash( src, QCryptographicHash::Md5 );
    return QString::fromLatin1( digest.toHex() ).rightJustified( 32, '0' );
}


GroovesharkServiceSettings::GroovesharkServiceSettings( QWidget *parent, const QVariantList &args )
    : KCModule( GroovesharkServiceSettingsFactory::componentData(), parent, args )
{
    debug() << "Creating Grooveshark config object";

    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget *w = new QWidget;
    m_configDialog = new Ui::GroovesharkConfigWidget;
    m_configDialog->setupUi( w );
    l->addWidget( w );

    // whenever this gets opened, we'll assume the user wants to change something,
    // so blow away the saved session key
    m_config.clearSessionKey();

    connect( m_configDialog->kcfg_ScrobblerUsername, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( m_configDialog->kcfg_ScrobblerPassword, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( m_configDialog->kcfg_SubmitPlayedSongs, SIGNAL( stateChanged( int ) ), this, SLOT( settingsChanged() ) );
    connect( m_configDialog->kcfg_RetrieveSimilarArtists, SIGNAL( stateChanged( int ) ), this, SLOT( settingsChanged() ) );
    connect( m_configDialog->testLogin, SIGNAL( clicked() ), this, SLOT( testLogin() ) );

    load();
}


GroovesharkServiceSettings::~GroovesharkServiceSettings()
{
}


void
GroovesharkServiceSettings::save()
{
    m_config.setUsername( m_configDialog->kcfg_ScrobblerUsername->text() );
    m_config.setPassword( m_configDialog->kcfg_ScrobblerPassword->text() );
    m_config.setScrobble( m_configDialog->kcfg_SubmitPlayedSongs->isChecked() );
    m_config.setFetchSimilar( m_configDialog->kcfg_RetrieveSimilarArtists->isChecked() );
    m_config.save();

    KCModule::save();
}

void
GroovesharkServiceSettings::testLogin()
{
    DEBUG_BLOCK

    m_configDialog->testLogin->setEnabled( false );
    m_configDialog->testLogin->setText( i18n( "Testing..." ) );
    // set the global static Grooveshark::Ws stuff
    /*
    grooveshark::ws::ApiKey = Amarok::groovesharkApiKey();
    grooveshark::ws::SharedSecret = "fe0dcde9fcd14c2d1d50665b646335e9";
    grooveshark::ws::Username = qstrdup( m_configDialog->kcfg_ScrobblerUsername->text().toLatin1().data() );
    if( grooveshark::nam() != The::networkAccessManager() )
        grooveshark::setNetworkAccessManager( The::networkAccessManager() );

    debug() << "username:" << QString( QUrl::toPercentEncoding( grooveshark::ws::Username ) );

    const QString authToken = md5( QString( "%1%2" ).arg( m_configDialog->kcfg_ScrobblerUsername->text() )
                                                    .arg( md5( m_configDialog->kcfg_ScrobblerPassword->text().toUtf8() ) ).toUtf8() );

    // now authenticate w/ Grooveshark and get our session key
    QMap<QString, QString> query;
    query[ "method" ] = "auth.getMobileSession";
    query[ "username" ] = m_configDialog->kcfg_ScrobblerUsername->text();
    query[ "authToken" ] = authToken;
    m_authQuery = grooveshark::ws::post( query );

    connect( m_authQuery, SIGNAL( finished() ), SLOT( onAuthenticated() ) );
    connect( m_authQuery, SIGNAL( error( QNetworkReply::NetworkError ) ), SLOT( onError( QNetworkReply::NetworkError ) ) );
    */
}

void
GroovesharkServiceSettings::onAuthenticated()
{
    DEBUG_BLOCK
    /*
    grooveshark::XmlQuery lfm = grooveshark::XmlQuery( m_authQuery->readAll() );

    switch( m_authQuery->error() )
    {
        case QNetworkReply::NoError:
             debug() << "NoError";
             if( lfm.children( "error" ).size() > 0 )
             {
                 debug() << "ERROR from Grooveshark:" << lfm.text();
                 m_configDialog->testLogin->setText( i18nc( "The operation was rejected by the server", "Failed" ) );
                 m_configDialog->testLogin->setEnabled( true );

             } else
             {
                 m_configDialog->testLogin->setText( i18nc( "The operation completed as expected", "Success" ) );
                 m_configDialog->testLogin->setEnabled( false );
                 m_configDialog->kcfg_SubmitPlayedSongs->setEnabled( true );
             }
             break;

        case QNetworkReply::AuthenticationRequiredError:
            debug() << "AuthenticationFailed";
            KMessageBox::error( this, i18n( "Either the username or the password is incorrect, please correct and try again" ), i18n( "Failed" ) );
            m_configDialog->testLogin->setText( i18n( "Test Login" ) );
            m_configDialog->testLogin->setEnabled( true );
            break;

        default:
            debug() << "Unhandled QNetworkReply state, probably not important";
    }
    m_authQuery->deleteLater();
    */
}

void
GroovesharkServiceSettings::onError( QNetworkReply::NetworkError code )
{
    DEBUG_BLOCK

    if( code == QNetworkReply::NoError )
        return;

    if( code == QNetworkReply::AuthenticationRequiredError )
    {
        onAuthenticated();
        return;
    }

KMessageBox::error( this, i18n( "Unable to connect to Grooveshark service." ), i18n( "Failed" ) );
    m_configDialog->testLogin->setText( i18n( "Test Login" ) );
    m_configDialog->testLogin->setEnabled( true );

    debug() << "Error occurred during network request: " << m_authQuery->errorString();
    m_authQuery->deleteLater();
}


void
GroovesharkServiceSettings::load()
{
    m_config.load();
    m_configDialog->kcfg_ScrobblerUsername->setText( m_config.username() );
    m_configDialog->kcfg_ScrobblerPassword->setText( m_config.password() );
    m_configDialog->kcfg_SubmitPlayedSongs->setChecked( m_config.scrobble() );
    m_configDialog->kcfg_RetrieveSimilarArtists->setChecked( m_config.fetchSimilar() );

    if( !m_config.username().isEmpty() && !m_config.password().isEmpty() )
        m_configDialog->kcfg_SubmitPlayedSongs->setEnabled( true );

    KCModule::load();
}


void
GroovesharkServiceSettings::defaults()
{
    m_config.reset();

    // By default this checkboxes is:
    m_configDialog->kcfg_SubmitPlayedSongs->setChecked( false );
    m_configDialog->kcfg_RetrieveSimilarArtists->setChecked( false );
}


void
GroovesharkServiceSettings::settingsChanged()
{
    //TODO: Make pretty validation for username and password
    //with error reporting

    m_configDialog->testLogin->setText( i18n( "&Test Login" ) );
    m_configDialog->testLogin->setEnabled( true );

    emit changed( true );
}
