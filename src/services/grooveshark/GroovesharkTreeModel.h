/****************************************************************************************
 * Copyright (c) 2009 Casey Link <unnamedrambler@gmail.com>                             *
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2009 Mark Kretschmann <kretschmann@kde.org>                            *
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

#ifndef GROOVESHARKTREEMODEL_H
#define GROOVESHARKTREEMODEL_H

#include "core/meta/Meta.h"
//#include "WeightedStringList.h"

//#include <grooveshark/User>

#include <QAbstractItemModel>
#include <QHash>
#include <QIcon>
#include <QMap>
#include <QModelIndex>
#include <QPixmap>
#include <QVariant>

class QNetworkReply;

namespace Grooveshark
{
enum Type
{
    Root = 0,
    MyRecommendations,
    PersonalRadio,
    MixRadio,
    NeighborhoodRadio,
    //         RecentlyPlayed,
    //         RecentlyLoved,
    //         RecentlyBanned,
    TopArtists,
    MyTags,
    Friends,
    Neighbors,

    //         History,

    RowCount,

    MyTagsChild,
    FriendsChild,
    NeighborsChild,
    ArtistsChild,
    RecentlyBannedTrack,
    RecentlyPlayedTrack,
    RecentlyLovedTrack,
    HistoryStation,

    UserChildPersonal,
    UserChildNeighborhood,

    TypeUnknown
};

enum Role
{
    StationUrlRole = Qt::UserRole,
    UrlRole,
    TrackRole,
    TypeRole
};

enum SortOrder
{
    MostWeightOrder,
    AscendingOrder,
    DescendingOrder
};


}

class GroovesharkTreeItem;
class KUrl;
class WsReply;


class GroovesharkTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GroovesharkTreeModel ( const QString &username, QObject *parent = 0 );
    ~GroovesharkTreeModel();

    QVariant data ( const QModelIndex &index, int role ) const;
    Qt::ItemFlags flags ( const QModelIndex &index ) const;
    QVariant headerData ( int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole ) const;
    QModelIndex index ( int row, int column,
                        const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent ( const QModelIndex &index ) const;
    int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex &parent = QModelIndex() ) const;
    int avatarSize () const;
    void prepareAvatar ( QPixmap& avatar, int size );

    void sortTags ( QString tagsToSort, Qt::SortOrder sortOrder );
    void sortTags ( Qt::SortOrder sortOrder );

    virtual QMimeData* mimeData( const QModelIndexList &indices ) const;

private slots:
    void onAvatarDownloaded( const QString& username, QPixmap );
    void slotAddNeighbors();
    void slotAddFriends();
    void slotAddTags();
    void slotAddTopArtists();

private:
    void setupModelData( GroovesharkTreeItem *parent );
    void emitRowChanged( int parent, int child = -1 );

    GroovesharkTreeItem *m_rootItem;
    GroovesharkTreeItem *m_myTags;
    GroovesharkTreeItem *m_myFriends;
    GroovesharkTreeItem *m_myNeighbors;
    GroovesharkTreeItem *m_myTopArtists;

    QString m_userName;
    //Grooveshark::AuthenticatedUser m_user;

    QStringList m_friends;
    QStringList m_neighbors;
    //WeightedStringList m_tags;

    QPixmap mAvatar;
    QMap<QString, QString> m_avatarQueue;
    QHash<QString, QIcon> m_avatars;
    int m_avatarSize;

    QMap< QString, QNetworkReply* > m_jobs;

    void queueAvatarsDownload ( const QMap<QString, QString>& urls );
    void downloadAvatar ( const QString& user, const KUrl& url );
    QString mapTypeToUrl ( Grooveshark::Type type, const QString &key = "" );

    void appendUserStations ( GroovesharkTreeItem* item, const QString& user );
};

class GroovesharkTreeItem
{
public:
    GroovesharkTreeItem ( const Grooveshark::Type &type, const QVariant &data, GroovesharkTreeItem *parent = 0 );
    GroovesharkTreeItem ( const QString &url, const Grooveshark::Type &type, const QVariant &data, GroovesharkTreeItem *parent = 0 );
    explicit GroovesharkTreeItem ( const Grooveshark::Type &type, GroovesharkTreeItem *parent = 0 );
    GroovesharkTreeItem ( const QString &url, const Grooveshark::Type &type, GroovesharkTreeItem *parent = 0 );
    ~GroovesharkTreeItem();

    void appendChild ( GroovesharkTreeItem *child );

    GroovesharkTreeItem *child ( int row );
    int childCount() const;
    int columnCount() const;
    QVariant data () const;
    int row() const;
    GroovesharkTreeItem *parent();
    Meta::TrackPtr track() const;
    Grooveshark::Type type() const
    {
        return mType;
    }

private:
    QList<GroovesharkTreeItem*> childItems;
    Grooveshark::Type mType;
    GroovesharkTreeItem *parentItem;
    QVariant itemData;
    QString mUrl;
};

#endif
