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

#ifndef MEMORYCOLLECTION_H
#define MEMORYCOLLECTION_H

#include "core/meta/Meta.h"

#include <QReadWriteLock>

//QMap is slower than QHash but the items are ordered by key
typedef QMap<QString, Meta::TrackPtr> TrackMap;
typedef QMap<QString, Meta::ArtistPtr> ArtistMap;
typedef QMap<QString, Meta::AlbumPtr> AlbumMap;
typedef QMap<QString, Meta::GenrePtr> GenreMap;
typedef QMap<QString, Meta::ComposerPtr> ComposerMap;
typedef QMap<int, Meta::YearPtr> YearMap;
typedef QMap<QString, Meta::LabelPtr> LabelMap;
typedef QHash<Meta::LabelPtr, Meta::TrackList> LabelToTrackMap;

namespace Collections {

class MemoryCollection
{
    public:
        void acquireReadLock() { m_readWriteLock.lockForRead(); }
        void releaseLock() { m_readWriteLock.unlock(); }
        void acquireWriteLock() { m_readWriteLock.lockForWrite(); }

        TrackMap trackMap() { return m_trackMap; }
        ArtistMap artistMap() { return m_artistMap; }
        AlbumMap albumMap() { return m_albumMap; }
        GenreMap genreMap() { return m_genreMap; }
        ComposerMap composerMap() { return m_composerMap; }
        YearMap yearMap() { return m_yearMap; }
        LabelMap labelMap() { return m_labelMap; }
        LabelToTrackMap labelToTrackMap() { return m_labelToTrackMap; }

        void setTrackMap( const TrackMap &map ) { m_trackMap = map; }
        void addTrack( Meta::TrackPtr trackPtr ) { m_trackMap.insert( trackPtr->uidUrl(), trackPtr ); }
        void setArtistMap( const ArtistMap &map ) { m_artistMap = map; }
        void addArtist( Meta::ArtistPtr artistPtr) { m_artistMap.insert( artistPtr->name(), artistPtr ); }
        void setAlbumMap( const AlbumMap &map ) { m_albumMap = map; }
        void addAlbum ( Meta::AlbumPtr albumPtr ) { m_albumMap.insert( albumPtr->name(), albumPtr ); }
        void setGenreMap( GenreMap map ) { m_genreMap = map; }
        void addGenre( Meta::GenrePtr genrePtr) { m_genreMap.insert( genrePtr->name(), genrePtr ); }
        void setComposerMap( const ComposerMap &map ) { m_composerMap = map; }
        void addComposer( Meta::ComposerPtr composerPtr ) { m_composerMap.insert( composerPtr->name(), composerPtr ); }
        void setYearMap( const YearMap &map ) { m_yearMap = map; }
        void addYear( Meta::YearPtr yearPtr ) { m_yearMap.insert( yearPtr->year(), yearPtr ); }
        void clearLabels()
        {
            m_labelMap = LabelMap();
            m_labelToTrackMap = LabelToTrackMap();
        }

        void addLabelToTrack( const Meta::LabelPtr &labelPtr, const Meta::TrackPtr &track )
        {
            m_labelMap.insert( labelPtr->name(), labelPtr );
            Meta::TrackList tracks;
            if( m_labelToTrackMap.contains( labelPtr ) )
            {
                tracks = m_labelToTrackMap.value( labelPtr );
            }
            tracks << track;
            m_labelToTrackMap.insert( labelPtr, tracks );
        }


    protected:
        QReadWriteLock m_readWriteLock;
        TrackMap m_trackMap;
        ArtistMap m_artistMap;
        AlbumMap m_albumMap;
        GenreMap m_genreMap;
        ComposerMap m_composerMap;
        YearMap m_yearMap;
        LabelMap m_labelMap;
        LabelToTrackMap m_labelToTrackMap;

};

} //namespace Collections

#endif
