/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#include "GroovesharkMeta.h"
#include "GroovesharkStore.h"

#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "GroovesharkActions.h"
#include "GroovesharkConfig.h"
#include "core-impl/statistics/providers/url/PermanentUrlStatisticsProvider.h"
#include "SvgHandler.h"

#include <KLocale>
#include <KStandardDirs>

#include <QObject>

using namespace Meta;
using namespace Statistics;

GroovesharkMetaFactory::GroovesharkMetaFactory( const QString & dbPrefix, GroovesharkStore * store )
    : ServiceMetaFactory( dbPrefix )
    , m_membershipPrefix( QString() )
    , m_streamType( OGG )
    , m_userName( QString() )
    , m_password( QString() )
    , m_store( store )
{}

void GroovesharkMetaFactory::setMembershipInfo( const QString &prefix, const QString &userName, const QString &password )
{
    m_membershipPrefix = prefix;
    m_userName = userName;
    m_password = password;
}

void GroovesharkMetaFactory::setStreamType(int type)
{
    m_streamType = type;
}


int GroovesharkMetaFactory::getTrackSqlRowCount()
{
    debug() << "GroovesharkMetaFactory::getTrackSqlRowCount()";
   //return ServiceMetaFactory::getTrackSqlRowCount() + 2;
    return 1;
}

QString GroovesharkMetaFactory::getTrackSqlRows()
{
    debug() << "GroovesharkMetaFactory::getTrackSqlRows()";
//     QString sqlRows = ServiceMetaFactory::getTrackSqlRows();
// 
//     sqlRows += ", ";
//     sqlRows += tablePrefix() + "_tracks.preview_lofi, ";
//     sqlRows += tablePrefix() + "_tracks.preview_ogg ";
// 
//     return sqlRows;

    return "";
}

TrackPtr GroovesharkMetaFactory::createTrack(const QStringList & rows)
{
    GroovesharkTrack * track = new GroovesharkTrack( rows );

    if ( m_streamType == OGG ) {
        track->setUidUrl( track->oggUrl() );
    } else if (  m_streamType == LOFI ) {
        track->setUidUrl( track->lofiUrl() );
    }
    track->setStatisticsProvider( new PermanentUrlStatisticsProvider( track->uidUrl() ) );

    if ( !m_membershipPrefix.isEmpty() ) {
        QString url = track->uidUrl();
        url.replace( "http://he3.", "http://" + m_userName + ":" + m_password + "@" + m_membershipPrefix + "." );
        
        if ( m_streamType == MP3 ) {
            url.replace( ".mp3", "_nospeech.mp3" );
        }  else if ( m_streamType == OGG ) {
            url.replace( ".ogg", "_nospeech.ogg" );
        }
        
        track->setUidUrl( url );
        
        if ( m_membershipPrefix == "download" )
            track->setDownloadMembership();
    }

    return TrackPtr( track );
}

int GroovesharkMetaFactory::getAlbumSqlRowCount()
{
    debug() << "GroovesharkMetaFactory::getAlbumSqlRowCount()";
    //return ServiceMetaFactory::getAlbumSqlRowCount() + 3;
    return 2;
}

QString GroovesharkMetaFactory::getAlbumSqlRows()
{
//     QString sqlRows = ServiceMetaFactory::getAlbumSqlRows();
// 
//     sqlRows += ", ";
//     sqlRows += tablePrefix() + "_albums.cover_url, ";
//     sqlRows += tablePrefix() + "_albums.year, ";
//     sqlRows += tablePrefix() + "_albums.album_code ";
// 
// 
//     return sqlRows;

    return "";
}

AlbumPtr GroovesharkMetaFactory::createAlbum(const QStringList & rows)
{
    GroovesharkAlbum * album = new GroovesharkAlbum( rows );
    album->setStore( m_store );

    if ( m_membershipPrefix == "download" )
        album->setDownloadMembership();

    album->setSourceName( "Grooveshark.com" );
    return AlbumPtr( album );
}

int GroovesharkMetaFactory::getArtistSqlRowCount()
{
    debug() << "GroovesharkMetaFactory::getArtistSqlRowCount()";
    //return ServiceMetaFactory::getArtistSqlRowCount() + 2;
    return 3;
}

QString GroovesharkMetaFactory::getArtistSqlRows()
{
//     QString sqlRows = ServiceMetaFactory::getArtistSqlRows();
// 
//     sqlRows += ", ";
//     sqlRows += tablePrefix() + "_artists.photo_url, ";
//     sqlRows += tablePrefix() + "_artists.artist_page ";
// 
//     return sqlRows;
    return "";
}

ArtistPtr GroovesharkMetaFactory::createArtist(const QStringList & rows)
{
    GroovesharkArtist * artist = new GroovesharkArtist( rows );
    artist->setSourceName( "Grooveshark.com" );
    return ArtistPtr( artist );

    
}

GenrePtr GroovesharkMetaFactory::createGenre(const QStringList & rows)
{
    GroovesharkGenre * genre = new GroovesharkGenre( rows );
    genre->setSourceName( "Grooveshark.com" );
    return GenrePtr( genre );
}


///////////////////////////////////////////////////////////////////////////////
// class GroovesharkTrack
///////////////////////////////////////////////////////////////////////////////

GroovesharkTrack::GroovesharkTrack( const QString &name )
    : ServiceTrack( name )
    , m_downloadMembership ( false )
    , m_downloadAction( 0 )
{}

GroovesharkTrack::GroovesharkTrack(const QStringList & resultRow)
    : ServiceTrack( resultRow )
    , m_downloadMembership ( false )
    , m_downloadAction( 0 )
{
    m_lofiUrl = resultRow[7];
    m_oggUrl = resultRow[8];
}

QString GroovesharkTrack::lofiUrl()
{
    return m_lofiUrl;
}

void GroovesharkTrack::setLofiUrl(const QString & url)
{
    m_lofiUrl = url;
}

QString Meta::GroovesharkTrack::oggUrl() const
{
    return m_oggUrl;
}

void Meta::GroovesharkTrack::setOggUrl( const QString& url )
{
    m_oggUrl = url;
}

void Meta::GroovesharkTrack::setDownloadMembership()
{
    m_downloadMembership = true;
}


QList< QAction * > Meta::GroovesharkTrack::customActions()
{
    DEBUG_BLOCK
    QList< QAction * > actions;

    if ( !m_downloadAction ) {

        QString text = i18n( "&Download Album" );
        GroovesharkAlbum * mAlbum = dynamic_cast<GroovesharkAlbum *> ( album().data() );
        if ( mAlbum ) {
            m_downloadAction = new GroovesharkDownloadAction( text, mAlbum );
        }
    }

    if ( m_downloadAction && m_downloadMembership )
        actions.append( m_downloadAction );

    return actions;

}

QList< QAction * > Meta::GroovesharkTrack::currentTrackActions()
{

    DEBUG_BLOCK
    QList< QAction * > actions;

    if ( !m_downloadAction ) {

        QString text = i18n( "Grooveshark.com: &Download Album" );

        GroovesharkAlbum * mAlbum = dynamic_cast<GroovesharkAlbum *> ( album().data() );
        if ( mAlbum ) {
            m_downloadAction = new GroovesharkDownloadAction( text, mAlbum );
        }
    }

    if ( m_downloadAction && m_downloadMembership )
        actions.append( m_downloadAction );

    return actions;

}

QString Meta::GroovesharkTrack::sourceName()
{
    return "Grooveshark.com";
}

QString Meta::GroovesharkTrack::sourceDescription()
{
    return i18n( "The non evil record label that is fair to artists and customers alike" );
}

QPixmap Meta::GroovesharkTrack::emblem()
{
    return QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-grooveshark.png" ) );
}


QList< QString > Meta::GroovesharkTrack::moods()
{
    return m_moods;
}

void Meta::GroovesharkTrack::setMoods(QList< QString > moods)
{
    m_moods = moods;
}

void Meta::GroovesharkTrack::download()
{
    DEBUG_BLOCK
    GroovesharkAlbum * mAlbum = dynamic_cast<GroovesharkAlbum *> ( album().data() );
    if ( mAlbum )
        mAlbum->store()->download( this );
}

void Meta::GroovesharkTrack::setAlbumPtr( Meta::AlbumPtr album )
{
    ServiceTrack::setAlbumPtr( album );

    //get year from magnatue album:
    GroovesharkAlbum * ma = dynamic_cast<GroovesharkAlbum *>( album.data() );
    if ( ma )
    {
        YearPtr year = YearPtr( new ServiceYear( QString::number( ma->launchYear() ) ) );
        setYear( year );
    }
}


///////////////////////////////////////////////////////////////////////////////
// class GroovesharkArtist
///////////////////////////////////////////////////////////////////////////////

GroovesharkArtist::GroovesharkArtist( const QString &name )
    : ServiceArtist( name )
{}

GroovesharkArtist::GroovesharkArtist(const QStringList & resultRow)
    : ServiceArtist( resultRow )
{
    m_photoUrl = resultRow[3];
    m_groovesharkUrl = resultRow[4];
}

void GroovesharkArtist::setPhotoUrl( const QString &photoUrl )
{
    m_photoUrl = photoUrl;
}

QString GroovesharkArtist::photoUrl( ) const
{
    return m_photoUrl;
}

void GroovesharkArtist::setGroovesharkUrl( const QString & groovesharkUrl )
{
    m_groovesharkUrl = groovesharkUrl;
}

QString GroovesharkArtist::groovesharkUrl() const
{
    return m_groovesharkUrl;
}


///////////////////////////////////////////////////////////////////////////////
// class GroovesharkAlbum
///////////////////////////////////////////////////////////////////////////////

GroovesharkAlbum::GroovesharkAlbum( const QString &name )
    : ServiceAlbumWithCover( name )
    , m_coverUrl()
    , m_launchYear( 0 )
    , m_albumCode()
    , m_store( 0 )
    , m_downloadMembership( false )
    , m_downloadAction( 0 )
    , m_addToFavoritesAction( 0 )

{}

GroovesharkAlbum::GroovesharkAlbum(const QStringList & resultRow)
    : ServiceAlbumWithCover( resultRow )
    , m_downloadMembership ( false )
    , m_downloadAction( 0 )
    , m_addToFavoritesAction( 0 )
{
    m_coverUrl = resultRow[4];
    m_launchYear = resultRow[5].toInt();
    m_albumCode = resultRow[6];

    m_store = 0;
}

GroovesharkAlbum::~ GroovesharkAlbum()
{}


void GroovesharkAlbum::setLaunchYear( int launchYear )
{
    m_launchYear = launchYear;
}

int GroovesharkAlbum::launchYear( ) const
{
    return m_launchYear;
}

void GroovesharkAlbum::setAlbumCode(const QString & albumCode)
{
    m_albumCode = albumCode;
}

QString GroovesharkAlbum::albumCode()
{
    return m_albumCode;

}

void GroovesharkAlbum::setCoverUrl(const QString & coverUrl)
{
    m_coverUrl = coverUrl;
}

QString GroovesharkAlbum::coverUrl() const
{
    return m_coverUrl;
}

void Meta::GroovesharkAlbum::setStore(GroovesharkStore * store)
{
    m_store = store;
}

GroovesharkStore * Meta::GroovesharkAlbum::store()
{
    return m_store;
}

void Meta::GroovesharkAlbum::setDownloadMembership()
{
    DEBUG_BLOCK
    m_downloadMembership = true;
}

QList< QAction * > GroovesharkAlbum::customActions()
{
    DEBUG_BLOCK
    QList< QAction * > actions;

    if ( !m_downloadAction ) {

        QString text = i18n( "&Download Album" );
        m_downloadAction = new GroovesharkDownloadAction( text, this );
    }

    if ( !m_addToFavoritesAction )
    {
         QString text = i18n( "Add to Grooveshark.com &favorites" );
         m_addToFavoritesAction = new GroovesharkAddToFavoritesAction( text, this );
    }

    GroovesharkConfig config;
    if ( config.isMember() )
        actions.append( m_addToFavoritesAction );

    if ( m_downloadAction && config.isMember() && config.membershipType() == GroovesharkConfig::DOWNLOAD )
        actions.append( m_downloadAction );

    return actions;
}

void Meta::GroovesharkAlbum::download()
{
    DEBUG_BLOCK
    if ( store() )
        store()->download( this );
}

void Meta::GroovesharkAlbum::addToFavorites()
{
    DEBUG_BLOCK
    if ( store() )
        store()->addToFavorites( albumCode() );
}

///////////////////////////////////////////////////////////////////////////////
// class GroovesharkGenre
///////////////////////////////////////////////////////////////////////////////

GroovesharkGenre::GroovesharkGenre( const QString & name )
    : ServiceGenre( name )
{}

GroovesharkGenre::GroovesharkGenre( const QStringList & resultRow )
    : ServiceGenre( resultRow )
{}

#include "GroovesharkMeta.moc"
