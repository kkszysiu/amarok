/*
 *  Copyright (c) 2006-2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef AMAROK_COLLECTION_QUERYBUILDER_H
#define AMAROK_COLLECTION_QUERYBUILDER_H

#include "meta.h"

#include <QObject>
#include <QtGlobal>

class QueryBuilder : public QObject
{
    Q_OBJECT

    static const qint64 valUrl      = 1LL << 0;
    static const qint64 valTitle    = 1LL << 1;
    static const qint64 valArtist   = 1LL << 2;
    static const qint64 valALbum    = 1LL << 3;
    static const qint64 valGenre    = 1LL << 4;
    static const qint64 valComposer = 1LL << 5;
    static const qint64 valYear     = 1LL << 6;

    public:
        virtual QueryBuilder* reset() = 0;
        virtual void run() = 0;
        virtual void abortQuery() = 0;

        virtual QueryBuilder* startTrackQuery() = 0;
        virtual QueryBuilder* startArtistQuery() = 0;
        virtual QueryBuilder* startAlbumQuery() = 0;
        virtual QueryBuilder* startGenreQuery() = 0;
        virtual QueryBuilder* startComposerQuery() = 0;
        virtual QueryBuilder* startYearQuery() = 0;
        virtual QueryBuilder* startCustomQuery() = 0;

        /**
            only works after starting a custom query with startCustomQuery()
          */
        virtual QueryBuilder* addReturnValue( qint64 value ) = 0;
        virtual QueryBuilder* orderBy( qint64 value, bool descending = false ) = 0;

        virtual QueryBuilder* includeCollection( const QString &collectionId ) = 0;
        virtual QueryBuilder* excludeCollection( const QString &collectionId ) = 0;

        virtual QueryBuilder* addMatch( const TrackPtr &track ) = 0;
        virtual QueryBuilder* addMatch( const ArtistPtr &artist ) = 0;
        virtual QueryBuilder* addMatch( const AlbumPtr &album ) = 0;
        virtual QueryBuilder* addMatch( const ComposerPtr &composer ) = 0;
        virtual QueryBuilder* addMatch( const GenrePtr &genre ) = 0;
        virtual QueryBuilder* addMatch( const YearPtr &year ) = 0;

        virtual QueryBuilder* addFilter( qint64 value, const QString &filter );
        virtual QueryBuilder* excludeFilter( qint64 value, const Qstring &filter );

    public signals:
        void newResultReady( QString collectionId, TrackList );
        void newResultReady( QString collectionId, ArtistList );
        void newResultReady( QString collectionId, AlbumList );
        void newResultReady( QString collectionId, GenreList );
        void newResultReady( QString collectionId, ComposerList );
        void newResultReady( QString collectionId, YearList );
        void newResultReady( QString collectionId, QStringList );

        void queryDone();
};

#endif /* AMAROK_COLLECTION_QUERYBUILDER_H */

