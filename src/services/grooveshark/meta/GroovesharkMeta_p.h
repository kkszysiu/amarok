/****************************************************************************************
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#ifndef AMAROK_GROOVESHARKMETA_P_H
#define AMAROK_GROOVESHARKMETA_P_H

#include "core/support/Debug.h"

#include "core/support/Amarok.h"
#include "amarokconfig.h"
#include "core/meta/Meta.h"
#include "core/statistics/StatisticsProvider.h"
#include "core-impl/statistics/providers/tag/TagStatisticsProvider.h"

//#include <grooveshark/Track>
//#include <grooveshark/ws.h>
//#include <grooveshark/RadioTuner>
//#include <grooveshark/XmlQuery>

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <KStandardDirs>

#include <QDir>
#include <QImage>
#include <QList>
#include <QPixmap>
#include <QStringList>
#include <QNetworkReply>



namespace Grooveshark
{

class Track::Private : public QObject
{
    Q_OBJECT

    public:
        Track *t;
        //grooveshark::Track groovesharkTrack; // this is how we love, ban, etc
        QUrl trackPath;
        QUrl groovesharkUri;

        QImage albumArt;
        QString artist;
        QString album;
        QString track;
        qint64 length;

        //not sure what these are for but they exist in the GroovesharkBundle
        QString albumUrl;
        QString artistUrl;
        QString trackUrl;
        QString imageUrl;

        Meta::ArtistPtr artistPtr;
        Meta::AlbumPtr albumPtr;
        Meta::GenrePtr genrePtr;
        Meta::ComposerPtr composerPtr;
        Meta::YearPtr yearPtr;

        QNetworkReply* trackFetch;
        QNetworkReply* wsReply;

        Statistics::StatisticsProvider *statisticsProvider;
        uint currentTrackStartTime;

    public:
        Private()
            : groovesharkUri( QUrl() )
            , statisticsProvider( 0 )
            , currentTrackStartTime( 0 )
        {
            artist = QString ( "Grooveshark" );
        }

        ~Private()
        {
        }

        void notifyObservers();

        /*
        void setTrackInfo( const grooveshark::Track &trackInfo )
        {
            DEBUG_BLOCK
            bool newTrackInfo = artist != trackInfo.artist() ||
                                album != trackInfo.album() ||
                                track != trackInfo.title();


            groovesharkTrack = trackInfo;
            artist = trackInfo.artist();
            album = trackInfo.album();
            track = trackInfo.title();
            length = trackInfo.duration() * 1000;
            trackPath = trackInfo.url();

            // need to reset other items
            albumUrl = "";
            trackUrl = "";
            albumArt = QImage();

            if( newTrackInfo )
            {
                delete statisticsProvider;
                statisticsProvider = new TagStatisticsProvider( track, artist, album );
                currentTrackStartTime = QDateTime::currentDateTime().toTime_t();
            }

            notifyObservers();

            if( !trackInfo.isNull() )
            {
                QMap< QString, QString > params;
                params[ "method" ] = "track.getInfo";
                params[ "artist" ] = artist;
                params[ "track" ] = track;

                //m_userFetch = grooveshark::ws::post( params );

                //connect( m_userFetch, SIGNAL( finished() ), SLOT( requestResult() ) );
            }
        }
    */

    public slots:
        void requestResult( )
        {
            if( !m_userFetch )
                return;
            if( m_userFetch->error() == QNetworkReply::NoError )
            {
                /*
                try
                {
                    grooveshark::XmlQuery lfm( m_userFetch->readAll() );
                    albumUrl = lfm[ "track" ][ "album" ][ "url" ].text();
                    trackUrl = lfm[ "track" ][ "url" ].text();
                    artistUrl = lfm[ "track" ][ "artist" ][ "url" ].text();

                    notifyObservers();

                    imageUrl = lfm[ "track" ][ "album" ][ "image size=large" ].text();

                    if( !imageUrl.isEmpty() )
                    {
                        KIO::Job* job = KIO::storedGet( KUrl( imageUrl ), KIO::Reload, KIO::HideProgressInfo );
                        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchImageFinished( KJob* ) ) );
                    }

                } catch( grooveshark::ws::ParseError& e )
                {
                    debug() << "Got exception in parsing from Grooveshark:" << e.what();
                }
                */
            }

        }

        void fetchImageFinished( KJob* job )
        {
            if( job->error() == 0 )
            {
                const int size = 100;

                QImage img = QImage::fromData( static_cast<KIO::StoredTransferJob*>( job )->data() );
                if( !img.isNull() )
                {
                    img.scaled( size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );

                    albumArt = img;
                }
                else
                    albumArt = QImage();
            }
            else
            {
                //use default image
                albumArt = QImage();
            }
            notifyObservers();
        }

    private:
        QNetworkReply* m_userFetch;

};

// internal helper classes

class GroovesharkArtist : public Meta::Artist
{
public:
    GroovesharkArtist( Track::Private *dptr )
        : Meta::Artist()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        if( d )
            return d->artist;
        return QString( "Grooveshark" );
    }

    Track::Private * const d;

    friend class Track::Private;
};

class GroovesharkAlbum : public Meta::Album
{
public:
    GroovesharkAlbum( Track::Private *dptr )
        : Meta::Album()
        , d( dptr )
    {}

    bool isCompilation() const { return false; }
    bool hasAlbumArtist() const { return false; }
    Meta::ArtistPtr albumArtist() const { return Meta::ArtistPtr(); }

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        if( d )
            return d->album;
        return QString();
    }

    QImage image( int size ) const
    {
        if( !d || d->albumArt.isNull() )
        {
            //return Meta::Album::image( size, withShadow );
            //TODO implement shadow
            //TODO improve this

            if ( size <= 1 )
                size = 100;
            QString sizeKey = QString::number( size ) + '@';

            QImage image;
            QDir cacheCoverDir = QDir( Amarok::saveLocation( "albumcovers/cache/" ) );
            if( cacheCoverDir.exists( sizeKey + "grooveshark-default-cover.png" ) )
                image = QImage( cacheCoverDir.filePath( sizeKey + "grooveshark-default-cover.png" ) );
            else
            {
                QImage orgImage = QImage( KStandardDirs::locate( "data", "amarok/images/grooveshark-default-cover.png" ) ); //optimize this!
                //scaled() does not change the original image but returns a scaled copy
                image = orgImage.scaled( size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
                image.save( cacheCoverDir.filePath( sizeKey + "grooveshark-default-cover.png" ), "PNG" );
            }

            return image;
        }


        if( d->albumArt.width() != size && size > 0 )
            return d->albumArt.scaled( size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
        return d->albumArt;
    }

    KUrl imageLocation( int size )
    {
        Q_UNUSED( size );
        if( d && !d->imageUrl.isEmpty() )
            return KUrl( d->imageUrl );
        return KUrl();
    }

    // return true since we handle our own fetching
    bool hasImage( int size = 1 ) const { Q_UNUSED( size ); return true; }

    Track::Private * const d;

    friend class Track::Private;
};

class GroovesharkGenre : public Meta::Genre
{
public:
    GroovesharkGenre( Track::Private *dptr )
        : Meta::Genre()
        , d( dptr )
    {}

    QString name() const
    {
        return QString();
    }

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    Track::Private * const d;

    friend class Track::Private;
};

class GroovesharkComposer : public Meta::Composer
{
public:
    GroovesharkComposer( Track::Private *dptr )
        : Meta::Composer()
        , d( dptr )
    {}

    QString name() const
    {
        return QString();
    }

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    Track::Private * const d;

    friend class Track::Private;
};

class GroovesharkYear : public Meta::Year
{
public:
    GroovesharkYear( Track::Private *dptr )
        : Meta::Year()
        , d( dptr )
    {}

    QString name() const
    {
        return QString();
    }

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    Track::Private * const d;

    friend class Track::Private;
};

void
Track::Private::notifyObservers()
{
    // TODO: only notify what actually has changed
    t->notifyObservers();
    static_cast<GroovesharkAlbum *>( t->album().data() )->notifyObservers();
    static_cast<GroovesharkArtist *>( t->artist().data() )->notifyObservers();
}

}

#endif
