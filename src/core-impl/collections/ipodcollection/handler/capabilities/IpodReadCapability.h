/****************************************************************************************
 * Copyright (c) 2009 Alejandro Wainzinger <aikawarazuni@gmail.com>                     *
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

#ifndef IPODREADCAPABILITY_H
#define IPODREADCAPABILITY_H

#include "ReadCapability.h"

namespace Meta {
    class IpodHandler;
}

namespace Handler
{

class IpodReadCapability : public ReadCapability
{
        Q_OBJECT

    public:
        IpodReadCapability( Meta::IpodHandler *handler );

        virtual void prepareToParseTracks();

        virtual bool isEndOfParseTracksList();

        virtual void prepareToParseNextTrack();

        virtual void nextTrackToParse();

        virtual void setAssociateTrack( const Meta::MediaDeviceTrackPtr track );

        virtual QString libGetTitle( const Meta::MediaDeviceTrackPtr &track );
        virtual QString libGetAlbum( const Meta::MediaDeviceTrackPtr &track );
        virtual QString libGetArtist( const Meta::MediaDeviceTrackPtr &track );
        virtual QString libGetAlbumArtist( const Meta::MediaDeviceTrackPtr &track );
        virtual QString libGetComposer( const Meta::MediaDeviceTrackPtr &track );
        virtual QString libGetGenre( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetYear( const Meta::MediaDeviceTrackPtr &track );
        virtual qint64  libGetLength( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetTrackNumber( const Meta::MediaDeviceTrackPtr &track );
        virtual QString libGetComment( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetDiscNumber( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetBitrate( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetSamplerate( const Meta::MediaDeviceTrackPtr &track );
        virtual qreal   libGetBpm( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetFileSize( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetPlayCount( const Meta::MediaDeviceTrackPtr &track );
        virtual QDateTime libGetLastPlayed( const Meta::MediaDeviceTrackPtr &track );
        virtual int     libGetRating( const Meta::MediaDeviceTrackPtr &track );
        virtual QString libGetType( const Meta::MediaDeviceTrackPtr &track );
        virtual KUrl libGetPlayableUrl( const Meta::MediaDeviceTrackPtr &track );

        virtual float usedCapacity() const;
        virtual float totalCapacity() const;

    private:
        Meta::IpodHandler *m_handler;
};

}

#endif
