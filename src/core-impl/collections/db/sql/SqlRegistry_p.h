/****************************************************************************************
 * Copyright (c) 2010 Ralf Engels <ralf-engels@gmx.de>                                  *
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

#ifndef SQLREGISTRY_P_H
#define SQLREGISTRY_P_H

#include "SqlMeta.h"

#include <QList>
#include <QString>

class SqlStorage;

/** A table committer will commit one table for a list of tracks.
    The table committer is a helper class that will batch insert/update one table
    for a list of tracks.
*/
class AbstractTrackTableCommitter
{
    public:
        /** Will commit one table for all the tracks.
          This will insert all the values into one table or update all the values.
          The id will be updated.
          */
        void commit( const QList<Meta::SqlTrackPtr> &tracks );

    protected:
        virtual QString tableName() = 0;
        virtual int getId( Meta::SqlTrack *track ) = 0;
        virtual void setId( Meta::SqlTrack *track, int id ) = 0;
        virtual QStringList getFields() = 0;
        virtual QStringList getValues( Meta::SqlTrack *track ) = 0;

        QString nullString( const QString &str ) const;
        QString nullNumber( const qint64 number ) const;
        QString nullNumber( const int number ) const;
        QString nullNumber( const double number ) const;
        QString nullDate( const QDateTime &date ) const;
        QString escape( const QString &str ) const;

        SqlStorage *m_storage;
};

class TrackUrlsTableCommitter: public AbstractTrackTableCommitter
{
    protected:
        virtual QString tableName();
        virtual int getId( Meta::SqlTrack *track );
        virtual void setId( Meta::SqlTrack *track, int id );
        virtual QStringList getFields();
        virtual QStringList getValues( Meta::SqlTrack *track );
};

class TrackTracksTableCommitter: public AbstractTrackTableCommitter
{
    protected:
        virtual QString tableName();
        virtual int getId( Meta::SqlTrack *track );
        virtual void setId( Meta::SqlTrack *track, int id );
        virtual QStringList getFields();
        virtual QStringList getValues( Meta::SqlTrack *track );
};

class TrackStatisticsTableCommitter: public AbstractTrackTableCommitter
{
    protected:
        virtual QString tableName();
        virtual int getId( Meta::SqlTrack *track );
        virtual void setId( Meta::SqlTrack *track, int id );
        virtual QStringList getFields();
        virtual QStringList getValues( Meta::SqlTrack *track );
};


#endif /* SQLREGISTRY_P_H */
