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

#ifndef AMAROK_GROOVESHARKMETA_H
#define AMAROK_GROOVESHARKMETA_H

#include "core/meta/Meta.h"
#include "core/capabilities/Capability.h"
#include "ServiceMetaBase.h" // for the SourceInfoProvider


//#include <grooveshark/Track>


#include <QObject>

class WsReply;

namespace Grooveshark
{
    class Track : public QObject, public Meta::Track, public SourceInfoProvider
    {
        Q_OBJECT
        public:
            class Private;

            Track( const QString &groovesharkUri );
            //Track( grooveshark::Track track ); //Convienience Constructor to allow constructing a Meta::GroovesharkTrack from a GroovesharkTrack (confusing?)
            virtual ~Track();

        //methods inherited from Meta::MetaBase
            virtual QString name() const;
            virtual QString fullPrettyName() const;
            virtual QString sortableName() const;
            virtual QString fixedName() const;
        //methods inherited from Meta::Track
            virtual KUrl playableUrl() const;
            virtual QString prettyUrl() const;
            virtual QString uidUrl() const;

            virtual bool isPlayable() const;

            virtual Meta::AlbumPtr album() const;
            virtual Meta::ArtistPtr artist() const;
            virtual Meta::GenrePtr genre() const;
            virtual Meta::ComposerPtr composer() const;
            virtual Meta::YearPtr year() const;

            virtual qreal bpm() const;

            virtual QString comment() const;

            virtual double score() const;
            virtual void setScore( double newScore );

            virtual int rating() const;
            virtual void setRating( int newRating );

            virtual int trackNumber() const;

            virtual int discNumber() const;

            virtual qint64 length() const;
            virtual int filesize() const;
            virtual int sampleRate() const;
            virtual int bitrate() const;
            virtual QDateTime lastPlayed() const;
            virtual QDateTime firstPlayed() const;
            virtual int playCount() const;

            virtual QString type() const;

            virtual void finishedPlaying( double playedFraction );

            virtual bool inCollection() const;
            virtual Collections::Collection *collection() const;

            virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;

            virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

            //void setTrackInfo( const grooveshark::Track &trackInfo );

            virtual QString sourceName();
            virtual QString sourceDescription();
            virtual QPixmap emblem();
            virtual QString scalableEmblem();

            //Grooveshark specific methods, cast the object to Grooveshark::Track to use them
            //you can cast the Track when type() returns "stream/grooveshark" (or use a dynamic cast:)
            KUrl internalUrl() const; // this returns the private temporary url to the .mp3, DO NOT USE,
                                   // if you are asking, it has already expired
            QString streamName() const; // A nice name for the stream..
        public slots:
            void love();
            void ban();
            void skip();

        private slots:
            void slotResultReady();
            void slotWsReply();

        signals:
            void skipTrack(); // needed for communication with multiplayablecapability
        private:
            void init( int id = -1 );
            //use a d-pointer because some code is going to work directly with Grooveshark::Track
            Private * const d;
            QList< QAction * > m_trackActions;
    };

    class GroovesharkProviderCapability : public Capabilities::Capability
    {
        public:
            GroovesharkProviderCapability();
            ~GroovesharkProviderCapability();
    };

    typedef KSharedPtr<Track> TrackPtr;
}

#endif
