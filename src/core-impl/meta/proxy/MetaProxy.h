/****************************************************************************************
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#ifndef AMAROK_METAPROXY_H
#define AMAROK_METAPROXY_H

#include "core/meta/Meta.h"
#include "core/capabilities/Capability.h"

#include <QObject>

namespace Collections
{
    class TrackProvider;
}

namespace MetaProxy
{
    class Track;
    typedef KSharedPtr<Track> TrackPtr;
    class AMAROK_EXPORT Track : public Meta::Track
    {
        public:
            class Private;

            Track( const KUrl &url );
            virtual ~Track();

        //methods inherited from Meta::MetaBase
            virtual QString name() const;
            virtual void setName( const QString &name );
            virtual QString prettyName() const;
            virtual QString fullPrettyName() const;
            virtual QString sortableName() const;

        //methods inherited from Meta::Track
            virtual KUrl playableUrl() const;
            virtual QString prettyUrl() const;
            virtual QString uidUrl() const;
            virtual void setUidUrl( const QString &newUidUrl ) const;

            virtual bool isPlayable() const;

            virtual Meta::AlbumPtr album() const;
            virtual void setAlbum( const QString &album );
            virtual void setAlbumArtist( const QString &artist );
            virtual Meta::ArtistPtr artist() const;
            virtual void setArtist( const QString &artist );
            virtual Meta::GenrePtr genre() const;
            virtual void setGenre( const QString &genre );
            virtual Meta::ComposerPtr composer() const;
            virtual void setComposer( const QString &composer );
            virtual Meta::YearPtr year() const;
            virtual void setYear( int year );

            virtual qreal bpm() const;
            virtual void setBpm( const qreal bpm );

            virtual QString comment() const;

            virtual double score() const;
            virtual void setScore( double newScore );

            virtual int rating() const;
            virtual void setRating( int newRating );

            virtual int trackNumber() const;
            virtual void setTrackNumber( int number );
            virtual int discNumber() const;
            virtual void setDiscNumber( int discNumber );

            virtual qint64 length() const;
            virtual int filesize() const;
            virtual int sampleRate() const;
            virtual int bitrate() const;
            virtual QDateTime createDate() const;

            virtual QDateTime firstPlayed() const;
            virtual QDateTime lastPlayed() const;
            virtual int playCount() const;

            virtual QString type() const;

            virtual void finishedPlaying( double playedFraction );

            virtual bool inCollection() const;
            virtual Collections::Collection *collection() const;

            virtual void subscribe( Meta::Observer *observer );
            virtual void unsubscribe( Meta::Observer *observer );

            virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;
            virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

            virtual bool operator==( const Meta::Track &track ) const;

		/**
		 * allows subclasses to create an instance of trackprovider which will only check the TrackProvider
		 * passed to lookupTrack(TrackProvider*) for the real track.
		 */
		Track( const KUrl &url, bool awaitLookupNotification);
		/**
		 * MetaProxy will check the given trackprovider if it can provide the track for the proxy's url.
		 */
		void lookupTrack(Collections::TrackProvider *provider);

        /**
         * MetaProxy will update the proxy with the track.
         */
        void updateTrack( Meta::TrackPtr track );

        private:
			void init( const KUrl &url, bool awaitLookupNotification );
            Private * const d;
    };

}

#endif
