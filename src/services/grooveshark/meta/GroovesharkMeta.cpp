/****************************************************************************************
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 * Copyright (c) 2008 Shane King <kde@dontletsstart.com>                                *
 * Copyright (c) 2008 Leo Franchi <lfranchi@kde.org>                                    *
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
#include "GroovesharkMeta_p.h"
#include "GroovesharkMeta_p.moc"
#include "GroovesharkCapabilityImpl_p.h"
#include "GroovesharkCapabilityImpl_p.moc"
#include "MultiPlayableCapabilityImpl_p.h"
#include "MultiPlayableCapabilityImpl_p.moc"
#include "ServiceCapabilities.h"

#include "GroovesharkService.h"
#include "GroovesharkStreamInfoCapability.h"
//#include "ScrobblerAdapter.h"

#include "EngineController.h"
#include "core/support/Debug.h"
#include "core/capabilities/ActionsCapability.h"

#include <KLocale>
#include <KSharedPtr>
#include <KStandardDirs>

#include <QWeakPointer>
#include <QUrl>

//#include <grooveshark/Track>

namespace Grooveshark {

class GroovesharkArtist;
class GroovesharkAlbum;
class GroovesharkGenre;
class GroovesharkComposer;
class GroovesharkYear;

Track::Track( const QString &groovesharkUri )
    : QObject()
    , Meta::Track()
    , d( new Private() )
{
    d->groovesharkUri = QUrl( groovesharkUri );
    d->t = this;


    init();
}

/*
Track::Track( grooveshark::Track track )
    : QObject()
    , Meta::Track()
    , d( new Private() )
{
    d->t = this;
    d->track = track.title();
    d->groovesharkTrack = track;
    QMap< QString, QString > params;
    params[ "method" ] = "track.getInfo";
    params[ "artist" ] = track.artist();
    params[ "track" ]  = track.title();

    d->trackFetch = grooveshark::ws::post( params );

    connect( d->trackFetch, SIGNAL( finished() ), SLOT( slotResultReady() ) );
}
*/


Track::~Track()
{
    delete d;
}

void Track::init( int id /* = -1*/ )
{
    if( id != -1 )
        d->groovesharkUri = QUrl( "grooveshark://play/tracks/" + QString::number( id ) );
    d->length = 0;

    d->albumPtr = Meta::AlbumPtr( new GroovesharkAlbum( d ) );
    d->artistPtr = Meta::ArtistPtr( new GroovesharkArtist( d ) );
    d->genrePtr = Meta::GenrePtr( new GroovesharkGenre( d ) );
    d->composerPtr = Meta::ComposerPtr( new GroovesharkComposer( d ) );
    d->yearPtr = Meta::YearPtr( new GroovesharkYear( d ) );

    QAction * banAction = new QAction( KIcon( "remove-amarok" ), i18n( "Grooveshark: &Ban" ), this );
    banAction->setShortcut( i18n( "Ctrl+B" ) );
    banAction->setStatusTip( i18n( "Ban this track" ) );
    connect( banAction, SIGNAL( triggered() ), this, SLOT( ban() ) );
    m_trackActions.append( banAction );

    QAction * skipAction = new QAction( KIcon( "media-seek-forward-amarok" ), i18n( "Grooveshark: &Skip" ), this );
    skipAction->setShortcut( i18n( "Ctrl+S" ) );
    skipAction->setStatusTip( i18n( "Skip this track" ) );
    connect( skipAction, SIGNAL( triggered() ), this, SLOT( skip() ) );
    m_trackActions.append( skipAction );
}

QString
Track::name() const
{
    if( d->track.isEmpty() )
    {
        return streamName();
    }
    else
    {
        return d->track;
    }
}

QString
Track::fullPrettyName() const
{
    if( d->track.isEmpty() || d->artist.isEmpty() )
        return prettyName();
    else
        return i18n("%1 - %2", d->artist, d->track );
}

QString
Track::sortableName() const
{
    // TODO
    return name();
}

QString Grooveshark::Track::fixedName() const
{

    //unless this is a stream for a single track, return the generic name
    if ( streamName() != i18n( "Track Radio" ) )
        return streamName();
    else
        return name();
}


KUrl
Track::playableUrl() const
{
    return d->groovesharkUri.toString();
}

KUrl
Track::internalUrl() const
{
    return KUrl( d->trackPath );
}

QString
Track::prettyUrl() const
{
    return d->groovesharkUri.toString();
}

QString
Track::uidUrl() const
{
    return d->groovesharkUri.toString();
}

bool
Track::isPlayable() const
{
    //we could check connectivity here...
    return !d->trackPath.isEmpty();
}

Meta::AlbumPtr
Track::album() const
{
    return d->albumPtr;
}

Meta::ArtistPtr
Track::artist() const
{
    return d->artistPtr;
}

Meta::GenrePtr
Track::genre() const
{
    return d->genrePtr;
}

Meta::ComposerPtr
Track::composer() const
{
    return d->composerPtr;
}

Meta::YearPtr
Track::year() const
{
    return d->yearPtr;
}

qreal
Track::bpm() const
{
    return -1.0;
}

QString
Track::comment() const
{
    return QString();
}

double
Track::score() const
{
    if( d->statisticsProvider )
        return d->statisticsProvider->score();
    else
        return 0.0;
}

void
Track::setScore( double newScore )
{
    if( d->statisticsProvider )
    {
        d->statisticsProvider->setScore( newScore );
        notifyObservers();
    }
}

int
Track::rating() const
{
    if( d->statisticsProvider )
        return d->statisticsProvider->rating();
    else
        return 0;
}

void
Track::setRating( int newRating )
{
    if( d->statisticsProvider )
    {
        d->statisticsProvider->setRating( newRating );
        notifyObservers();
    }
}

int
Track::trackNumber() const
{
    return 0;
}

int
Track::discNumber() const
{
    return 0;
}

qint64
Track::length() const
{
    return d->length;
}

int
Track::filesize() const
{
    return 0; //stream
}

int
Track::sampleRate() const
{
    return 0; //does the engine deliver this?
}

int
Track::bitrate() const
{
    return 0; //does the engine deliver this??
}

QDateTime
Track::lastPlayed() const
{
    if( d->statisticsProvider )
        return d->statisticsProvider->lastPlayed();
    else
        return QDateTime();
}

QDateTime
Track::firstPlayed() const
{
    if( d->statisticsProvider )
        return d->statisticsProvider->firstPlayed();
    else
        return QDateTime();
}

int
Track::playCount() const
{
    if( d->statisticsProvider )
        return d->statisticsProvider->playCount();
    return 0;
}

QString
Track::type() const
{
    return "stream/grooveshark";
}
void
Track::finishedPlaying( double playedFraction )
{
    if( d->statisticsProvider )
    {
        d->statisticsProvider->played( playedFraction, Meta::TrackPtr( this ) );
        notifyObservers();
    }
}

bool
Track::inCollection() const
{
    return false;
}

Collections::Collection*
Track::collection() const
{
    return 0;
}

/*
void
Track::setTrackInfo( const grooveshark::Track &track )
{
    if( !track.isNull() )
        d->setTrackInfo( track );
}
*/

QString
Track::streamName() const
{
    // parse the url to get a name if we don't have a track name (ie we're not playing the station)
    // do it as name rather than prettyname so it shows up nice in the playlist.
    QStringList elements = d->groovesharkUri.toString().split( '/', QString::SkipEmptyParts );
    if( elements.size() >= 2 && elements[0] == "grooveshark:" )
    {
        QString customPart = QUrl::fromPercentEncoding( elements[2].toUtf8() );

        if( elements[1] == "globaltags" )
        {
                // grooveshark://globaltag/<tag>
            if( elements.size() >= 3 )
                return i18n( "Global Tag Radio: \"%1\"", customPart );
        }
        else if( elements[1] == "usertags" )
        {
                // grooveshark://usertag/<tag>
            if( elements.size() >= 3 )
                return i18n( "User Tag Radio: \"%1\"", customPart );
        }
        else if( elements[1] == "artist" )
        {
            if( elements.size() >= 4 )
            {
                    // grooveshark://artist/<artist>/similarartists
                if( elements[3] == "similarartists" )
                    return i18n( "Similar Artists to \"%1\"", customPart );

                    // grooveshark://artist/<artist>/fans
                else if( elements[3] == "fans" )
                    return i18n( "Artist Fan Radio: \"%1\"", customPart );
            }
        }
        else if( elements[1] == "user" )
        {
            if( elements.size() >= 4 )
            {
                // grooveshark://user/<user>/neighbours
                if( elements[3] == "neighbours" )
                    return i18n( "%1's Neighbor Radio", elements[2] );

                // grooveshark://user/<user>/personal
                else if( elements[3] == "personal" )
                    return i18n( "%1's Personal Radio", elements[2] );

                // grooveshark://user/<user>/mix
                else if( elements[3] == "mix" )
                    return i18n( "%1's Mix Radio", elements[2] );

                // grooveshark://user/<user>/recommended
                else if( elements.size() < 5 && elements[3] == "recommended" )
                    return i18n( "%1's Recommended Radio", elements[2] );

                // grooveshark://user/<user>/recommended/<popularity>
                else if( elements.size() >= 5 && elements[3] == "recommended" )
                    return i18n( "%1's Recommended Radio (Popularity %2)", elements[2], elements[4] );
            }
        }
        else if( elements[1] == "group" )
        {
                // grooveshark://group/<group>
            if( elements.size() >= 3 )
                return i18n( "Group Radio: %1", elements[2] );
        }
        else if( elements[1] == "play" )
        {
            if( elements.size() >= 4 )
            {
                    // grooveshark://play/tracks/<track #s>
                if ( elements[2] == "tracks" )
                    return i18n( "Track Radio" );

                    // grooveshark://play/artists/<artist #s>
                else if ( elements[2] == "artists" )
                    return i18n( "Artist Radio" );
            }
        }
    }

    return d->groovesharkUri.toString();
}

void
Track::love()
{
    DEBUG_BLOCK

    //debug() << "info:" << d->groovesharkTrack.artist() << d->groovesharkTrack.title();
    //d->wsReply = grooveshark::MutableTrack( d->groovesharkTrack ).love();
    //connect( d->wsReply, SIGNAL( finished() ), this, SLOT( slotWsReply() ) );
}

void
Track::ban()
{
    DEBUG_BLOCK
    //d->wsReply = grooveshark::MutableTrack( d->groovesharkTrack ).ban();
    //connect( d->wsReply, SIGNAL( finished() ), this, SLOT( slotWsReply() ) );
    //if( The::engineController()->currentTrack() == this )
    //    emit( skipTrack() );
}

void
Track::skip()
{
    DEBUG_BLOCK
    //MutableTrack( d->groovesharkTrack ).skip();
    emit( skipTrack() );
}

void Track::slotResultReady()
{
    /*
    if( d->trackFetch->error() == QNetworkReply::NoError )
    {
        try
        {
            grooveshark::XmlQuery lfm( d->trackFetch->readAll() );
            QString id = lfm[ "track" ][ "id" ].text();
            QString streamable = lfm[ "track" ][ "streamable" ].text();
            if( streamable.toInt() == 1 )
                init( id.toInt() );
            else
                init();

        } catch( grooveshark::ws::ParseError& e )
        {
            debug() << "Got exception in parsing from Grooveshark:" << e.what();
        }
    } else
    {
        init();
    }
    d->trackFetch->deleteLater();
    */
}


void
Track::slotWsReply()
{
    if( d->wsReply->error() == QNetworkReply::NoError )
    {
        //debug() << "successfully completed WS transaction";
    } else
    {
        debug() << "ERROR in Grooveshark skip or ban!" << d->wsReply->error();
    }
}

bool
Track::hasCapabilityInterface( Capabilities::Capability::Type type ) const
{
    /*
    return type == Capabilities::Capability::Grooveshark
                || type == Capabilities::Capability::MultiPlayable
                || type == Capabilities::Capability::SourceInfo
                || type == Capabilities::Capability::Actions
                || type == Capabilities::Capability::StreamInfo;

    */
    return TRUE;
}

Capabilities::Capability*
Track::createCapabilityInterface( Capabilities::Capability::Type type )
{
    /*
    switch( type )
    {
        case Capabilities::Capability::Grooveshark:
            return new GroovesharkCapabilityImpl( this );
        //case Capabilities::Capability::MultiPlayable:
        //    return new MultiPlayableCapabilityImpl( this );
        case Capabilities::Capability::SourceInfo:
            return new ServiceSourceInfoCapability( this );
        case Capabilities::Capability::Actions:
            return new Capabilities::ActionsCapability( m_trackActions );
        case Capabilities::Capability::StreamInfo:
            return new GroovesharkStreamInfoCapability( this );
        default:
            return 0;
    }
    */
    return 0;
}

} // namespace Grooveshark

QString Grooveshark::Track::sourceName()
{
    return "Grooveshark";
}

QString Grooveshark::Track::sourceDescription()
{
    return i18n( "Grooveshark is cool..." );
}

QPixmap Grooveshark::Track::emblem()
{
    if (  !d->track.isEmpty() )
        return QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-grooveshark.png" ) );
    else
        return QPixmap();
}

QString Grooveshark::Track::scalableEmblem()
{
    if ( !d->track.isEmpty() )
        return KStandardDirs::locate( "data", "amarok/images/emblem-grooveshark-scalable.svg" );
    else
        return QString();
}

#include "GroovesharkMeta.moc"



