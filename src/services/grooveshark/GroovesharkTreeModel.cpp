/****************************************************************************************
 * Copyright (c) 2008 Casey Link <unnamedrambler@gmail.com>                             *
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

#define DEBUG_PREFIX "GroovesharkTreeModel"
#include "core/support/Debug.h"

#include "GroovesharkTreeModel.h"

#include "AvatarDownloader.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "AmarokMimeData.h"

//#include <grooveshark/ws.h>
//#include <grooveshark/Tag>
//#include <grooveshark/XmlQuery>

#include <KIcon>
#include <KLocale>

#include <QMap>
#include <QNetworkReply>
#include <QPainter>

using namespace Grooveshark;

GroovesharkTreeModel::GroovesharkTreeModel ( const QString &username, QObject *parent )
        : QAbstractItemModel ( parent ), m_userName ( username ), m_avatarSize( 32 )
//        : QAbstractItemModel ( parent ), m_userName ( username ), m_user(), m_avatarSize( 32 )
{
//     rootData << "Title" << "Summary";
    m_rootItem = new GroovesharkTreeItem ( Grooveshark::Root, "Hello" );
    setupModelData ( m_rootItem );
    /*
    m_jobs[ "getNeighbours" ] = m_user.getNeighbours();
    connect ( m_jobs[ "getNeighbours" ], SIGNAL ( finished () ), this, SLOT ( slotAddNeighbors () ) );
    
    m_jobs[ "getFriends" ] = m_user.getFriends();
    connect ( m_jobs[ "getFriends" ], SIGNAL ( finished () ), this, SLOT ( slotAddFriends () ) );
    
    m_jobs[ "getTopTags" ] = m_user.getTopTags();
    connect ( m_jobs[ "getTopTags" ], SIGNAL ( finished () ), this, SLOT ( slotAddTags () ) );
    
    m_jobs[ "getTopArtists" ] = m_user.getTopArtists();
    connect ( m_jobs[ "getTopArtists" ], SIGNAL ( finished () ), this, SLOT ( slotAddTopArtists () ) );
    */
}

GroovesharkTreeModel::~GroovesharkTreeModel()
{
    delete m_rootItem;
}

void
GroovesharkTreeModel::slotAddNeighbors ()
{
    DEBUG_BLOCK

    QMap<QString, QString> avatarlist;
    
    /*
    try
    {
        // Iterate over each neighbor, in two passes: 1) Get data 2) Sort data, store in model

        grooveshark::XmlQuery lfm( m_jobs[ "getNeighbours" ]->readAll() );
        foreach( const grooveshark::XmlQuery &e, lfm[ "neighbours" ].children ( "user" ) )
        {
            const QString name = e[ "name" ].text();
            m_neighbors << name;
            if ( !e[ "image size=small" ].text().isEmpty() )
            {
                avatarlist.insert ( name, e[ "image size=small" ].text() );
            }
        }

        m_neighbors.sort();

        foreach( const QString& name, m_neighbors )
        {
            GroovesharkTreeItem* neighbor = new GroovesharkTreeItem( mapTypeToUrl( Grooveshark::NeighborsChild, name ), Grooveshark::NeighborsChild, name, m_myNeighbors );
            m_myNeighbors->appendChild( neighbor );
            appendUserStations( neighbor, name );
        }
    }
    catch( grooveshark::ws::ParseError e )
    {
        debug() << "Got exception in parsing from Grooveshark:" << e.what();
    }
    queueAvatarsDownload ( avatarlist );
    emitRowChanged(Grooveshark::Neighbors);
    m_jobs[ "getNeighbours" ]->deleteLater();
    */
}

void
GroovesharkTreeModel::slotAddFriends ()
{
    DEBUG_BLOCK

    QMap<QString, QString> avatarlist;
    /*
    try
    {
        // Iterate over each friend, in two passes: 1) Get data 2) Sort data, store in model

        grooveshark::XmlQuery lfm( m_jobs[ "getFriends" ]->readAll() );
        foreach( const grooveshark::XmlQuery &e, lfm[ "friends" ].children ( "user" ) )
        {
            const QString name = e[ "name" ].text();
            m_friends << name;
            if( !e[ "image size=small" ].text().isEmpty() )
            {
                avatarlist.insert( name, e[ "image size=small" ].text() );
            }
        }

        m_friends.sort();

        foreach( const QString& name, m_friends )
        {
            GroovesharkTreeItem* afriend = new GroovesharkTreeItem( mapTypeToUrl ( Grooveshark::FriendsChild, name ), Grooveshark::FriendsChild, name, m_myFriends );
            m_myFriends->appendChild ( afriend );
            appendUserStations ( afriend, name );
        }
    }
    catch( grooveshark::ws::ParseError e )
    {
        debug() << "Got exception in parsing from Grooveshark:" << e.what();
    }
    queueAvatarsDownload ( avatarlist );
    emitRowChanged(Grooveshark::Friends);
    m_jobs[ "getFriends" ]->deleteLater();
    */
}

void
GroovesharkTreeModel::slotAddTopArtists ()
{
    DEBUG_BLOCK
    //WeightedStringList list;
    /*
    try
    {
        grooveshark::XmlQuery lfm( m_jobs[ "getTopArtists" ]->readAll() );

        foreach( const grooveshark::XmlQuery &e, lfm[ "topartists" ].children ( "artist" ) )
        {
            const QString name = e[ "name" ].text();
            const QString weight = e[ "playcount" ].text();
            //WeightedString s(name, weight.toFloat() );
            //list << s;
        }
        
        list.weightedSort(Qt::DescendingOrder);
        for ( int i = 0; i < list.count(); i++ )
        {
            list[i] += " (" + QVariant ( list.at ( i ).weighting() ).toString() + " plays)";
            QString actual = list[i];
            actual = actual.remove ( actual.lastIndexOf ( " (" ), actual.length() );
            GroovesharkTreeItem* artist = new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::ArtistsChild, actual ), Grooveshark::ArtistsChild, list[i], m_myTopArtists );
            m_myTopArtists->appendChild ( artist );
        }

    } catch( grooveshark::ws::ParseError e )
    {
        debug() << "Got exception in parsing from Grooveshark:" << e.what();
    }
    emitRowChanged(Grooveshark::TopArtists);
    m_jobs[ "getTopArtists" ]->deleteLater();
    */
}

void
GroovesharkTreeModel::appendUserStations ( GroovesharkTreeItem* item, const QString &user )
{
    GroovesharkTreeItem* personal = new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::UserChildPersonal, user ), Grooveshark::UserChildPersonal, i18n ( "Personal Radio" ), item );
    GroovesharkTreeItem* neigh = new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::UserChildNeighborhood, user ), Grooveshark::UserChildNeighborhood, i18n ( "Neighborhood" ), item );
    item->appendChild ( personal );
    item->appendChild ( neigh );
}
void
GroovesharkTreeModel::slotAddTags ()
{
    //DEBUG_BLOCK
    //m_tags.clear();
    //QMap< int, QString > listWithWeights = grooveshark::Tag::list ( m_jobs[ "getTopTags" ] );
    //WeightedStringList weighted;
    //foreach( int w, listWithWeights.keys() )
    //    weighted << WeightedString( listWithWeights[ w ], w );
    //sortTags ( weighted, Qt::DescendingOrder ) ;
    //emitRowChanged(Grooveshark::MyTags);
    //m_jobs[ "getTopTags" ]->deleteLater();
}

void
GroovesharkTreeModel::sortTags ( QString tagsToSort, Qt::SortOrder sortOrder )
{
    /*
    for ( int i = 0; i < tagsToSort.count(); i++ )
        tagsToSort[i] += " (" + QVariant ( tagsToSort.at ( i ).weighting() ).toString() + ')';
    tagsToSort.weightedSort ( sortOrder );
//     m_tags = tagsToSort;
    for ( int i = 0; i < tagsToSort.count(); i++ )
    {
        QString actual = tagsToSort[i];
        actual = actual.remove ( actual.lastIndexOf ( " (" ), actual.length() );
        GroovesharkTreeItem* tag = new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::MyTagsChild, actual ), Grooveshark::MyTagsChild, tagsToSort[i], m_myTags );
        m_myTags->appendChild ( tag );
    }
    */
}

void
GroovesharkTreeModel::sortTags ( Qt::SortOrder sortOrder )
{
    //sortTags ( m_tags, sortOrder );
}
/*
template <class T> void
GroovesharkTreeModel::changeData ( int row, T& old_data, const T& new_data )
{
    DEBUG_BLOCK
    QModelIndex const parent = index ( row, 0 );
    int const n = old_data.count() - new_data.count();
    if ( n > 0 ) beginRemoveRows ( parent, 0, n - 1 );
    if ( n < 0 ) beginInsertRows ( parent, 0, -n );
    old_data = new_data;
    if ( n > 0 ) endRemoveRows();
    if ( n < 0 ) endInsertRows();
    emit dataChanged( index( 0, 0, parent ), index( new_data.count() - 1, 0, parent ) );
}*/

void
GroovesharkTreeModel::emitRowChanged( int parent_row, int child_row )
{
    QModelIndex parent = index( parent_row, 0 );
    if( child_row != -1 )
    {
        QModelIndex i = index( child_row, 0, parent );
        emit dataChanged( i, i );
    }
    else
        emit dataChanged( parent, parent );
}


void
GroovesharkTreeModel::queueAvatarsDownload ( const QMap<QString, QString>& urls )
{
    bool start = m_avatarQueue.isEmpty();
    m_avatarQueue.unite ( urls );

    QMutableMapIterator<QString, QString> i ( m_avatarQueue );
    while ( i.hasNext() )
    {
        i.next();

        QString const name = i.key();
        QString const url = i.value();

        //         if ( !KUrl( url ).host().startsWith( USER_AVATAR_HOST ) )
        //         {
        // Don't download avatar if it's just the default blank avatar!
        // but do if it's the current username since we have to show something at the top there
        //             if ( name != m_username )
        //                 i.remove();
        //         }
    }

    if ( start )
        downloadAvatar ( m_avatarQueue.keys().value ( 0 ), m_avatarQueue.values().value ( 0 ) );
}


void
GroovesharkTreeModel::downloadAvatar ( const QString& user, const KUrl& url )
{
    //     debug() << "downloading " << user << "'s avatar @ "  << url;
    AvatarDownloader* downloader = new AvatarDownloader();
    downloader->downloadAvatar( user, url );
    connect( downloader, SIGNAL(avatarDownloaded(const QString&, QPixmap)),
                         SLOT(onAvatarDownloaded(const QString&, QPixmap)) );
}

void
GroovesharkTreeModel::prepareAvatar ( QPixmap& avatar, int size )
{
    // This code is here to stop Qt from crashing on certain weirdly shaped avatars.
    // We had a case were an avatar got a height of 1px after scaling and it would
    // crash in the rendering code. This here just fills in the background with
    // transparency first.
    if ( avatar.width() < size || avatar.height() < size )
    {
        QImage finalAvatar ( size, size, QImage::Format_ARGB32 );
        finalAvatar.fill ( 0 );

        QPainter p ( &finalAvatar );
        QRect r;

        if ( avatar.width() < size )
            r = QRect ( ( size - avatar.width() ) / 2, 0, avatar.width(), avatar.height() );
        else
            r = QRect ( 0, ( size - avatar.height() ) / 2, avatar.width(), avatar.height() );

        p.drawPixmap ( r, avatar );
        p.end();

        avatar = QPixmap::fromImage ( finalAvatar );
    }
}

void
GroovesharkTreeModel::onAvatarDownloaded( const QString &username, QPixmap avatar )
{
    //     DEBUG_BLOCK
    if( !avatar.isNull() && avatar.height() > 0 && avatar.width() > 0 )
    {
        int m = m_avatarSize;

        if( username != m_userName )
        {
            avatar = avatar.scaled ( m, m, Qt::KeepAspectRatio, Qt::SmoothTransformation );

            prepareAvatar( avatar, m );

            if ( !avatar.isNull() && avatar.height() > 0 && avatar.width() > 0 )
            {
                //                 debug() << "inserting avatar";
                m_avatars.insert ( username, avatar );
                emitRowChanged( Grooveshark::Friends );
                emitRowChanged( Grooveshark::Neighbors );
            }
        }
    }

    sender()->deleteLater();

    m_avatarQueue.remove ( username );
    if ( m_avatarQueue.count() )
        downloadAvatar ( m_avatarQueue.keys().value ( 0 ), m_avatarQueue.values().value ( 0 ) );
}

int GroovesharkTreeModel::columnCount ( const QModelIndex &parent ) const
{
    Q_UNUSED( parent )
    return 1;
}

int GroovesharkTreeModel::avatarSize () const
{
    return m_avatarSize;
}

QVariant GroovesharkTreeModel::data ( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    GroovesharkTreeItem *i = static_cast<GroovesharkTreeItem*> ( index.internalPointer() );
    if ( role == Qt::DisplayRole )
        switch ( i->type() )
        {
        case MyRecommendations:
            return i18n ( "My Recommendations" );
        case PersonalRadio:
            return i18n ( "My Radio Station" );
        case MixRadio:
            return i18n ( "My Mix Radio" );
        case NeighborhoodRadio:
            return i18n ( "My Neighborhood" );
            //             case RecentlyPlayed:      return tr("Recently Played");
            //             case RecentlyLoved:       return tr("Recently Loved");
            //             case RecentlyBanned:      return tr("Recently Banned");
        case TopArtists:
            return i18n ( "My Top Artists" );
        case MyTags:
            return i18n ( "My Tags" );
        case Friends:
            return i18n ( "Friends" );
        case Neighbors:
            return i18n ( "Neighbors" );
            //             case History:             return tr("History");

            //             case RecentlyPlayedTrack: return m_played.value( index.row() );
            //             case RecentlyLovedTrack:  return m_loved.value( index.row() );
            //             case RecentlyBannedTrack: return m_banned.value( index.row() );
//             case MyTagsChild:         return m_tags.value( index.row() );
        case FriendsChild:
        case ArtistsChild:
        case NeighborsChild:
        case UserChildPersonal:
        case UserChildNeighborhood:
        case MyTagsChild:
            return i->data();
        default:
            break;
        }

    if ( role == Qt::DecorationRole )
        switch ( i->type() )
        {
            //             case MyProfile:           return m_avatar;
        case MyRecommendations:
            return KIcon ( "grooveshark-recommended-radio-amarok" );
        case TopArtists:
        case PersonalRadio:
            return KIcon ( "grooveshark-personal-radio-amarok" );
        case MixRadio:
            return KIcon ( "grooveshark-mix-radio-amarok" );
        case NeighborhoodRadio:
            return KIcon ( "grooveshark-neighbour-radio-amarok" );
            //             case RecentlyPlayed:      return KIcon( "grooveshark-recent-tracks-amarok" );
            //             case RecentlyLoved:       return KIcon( "grooveshark-recently-loved-amarok" );
            //             case RecentlyBanned:      return KIcon( "grooveshark-recently-banned-amarok" );
        case MyTags:
            return KIcon ( "grooveshark-my-tags-amarok" );
        case Friends:
            return KIcon ( "grooveshark-my-friends-amarok" );
        case Neighbors:
            return KIcon ( "grooveshark-my-neighbours-amarok" );

        case RecentlyPlayedTrack: //FALL THROUGH
        case RecentlyLovedTrack:  //FALL THROUGH
        case RecentlyBannedTrack:
            return KIcon ( "icon_track" );
        case MyTagsChild:
            return KIcon ( "grooveshark-tag-amarok" );

        case FriendsChild:
        {
            if ( m_avatars.contains ( index.data().toString() ) )
                return m_avatars.value ( index.data().toString() );

            return KIcon ( "filename-artist-amarok" );
        }
        case UserChildPersonal:
            return KIcon ( "grooveshark-personal-radio-amarok" );
        case UserChildNeighborhood:
            return KIcon ( "grooveshark-neighbour-radio-amarok" );

        case NeighborsChild:
        {
            if ( m_avatars.contains ( index.data().toString() ) )
                return m_avatars.value ( index.data().toString() );

            return KIcon ( "filename-artist-amarok" );
        }

        case HistoryStation:
            return KIcon ( "icon_radio" );

        default:
            break;
        }

        if( role == Grooveshark::TrackRole )
        {
            switch ( i->type() )
            {
                case Grooveshark::MyRecommendations:
                case Grooveshark::PersonalRadio:
                case Grooveshark::MixRadio:
                case Grooveshark::NeighborhoodRadio:
                case Grooveshark::FriendsChild:
                case Grooveshark::NeighborsChild:
                case Grooveshark::MyTagsChild:
                case Grooveshark::ArtistsChild:
                case Grooveshark::UserChildPersonal:
                case Grooveshark::UserChildNeighborhood:
                    return QVariant::fromValue( i->track() );
                default:
                    break;
            }
        }
        if( role == Grooveshark::TypeRole )
            return i->type();

//     return i->data ( index.column() );
    return QVariant();
}

Qt::ItemFlags GroovesharkTreeModel::flags ( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    GroovesharkTreeItem *i = static_cast<GroovesharkTreeItem*> ( index.internalPointer() );
    switch ( i->type() )
    {
    case MyRecommendations:
    case PersonalRadio:
    case MixRadio:
    case NeighborhoodRadio:
    case RecentlyPlayedTrack:
    case RecentlyLovedTrack:
    case RecentlyBannedTrack:
    case MyTagsChild:
    case FriendsChild:
    case ArtistsChild:
    case NeighborsChild:
    case HistoryStation:
    case UserChildPersonal:
    case UserChildNeighborhood:
        flags |= Qt::ItemIsSelectable;
        break;

    default:
        break;
    }

    switch ( i->type() )
    {
    case UserChildPersonal:
    case UserChildNeighborhood:
    case MyTagsChild:
    case ArtistsChild:
    case MyRecommendations:
    case PersonalRadio:
    case MixRadio:
    case NeighborhoodRadio:
        flags |= Qt::ItemIsDragEnabled;

    default:
        break;
    }

    return flags;
}

QVariant GroovesharkTreeModel::headerData ( int section, Qt::Orientation orientation,
                                       int role ) const
{
    Q_UNUSED( section )
    Q_UNUSED( role )
    Q_UNUSED( orientation )
//     if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
//         return m_rootItem->data ( section );

    return QVariant();
}

QModelIndex GroovesharkTreeModel::index ( int row, int column, const QModelIndex &parent )
const
{
    if ( !hasIndex ( row, column, parent ) )
        return QModelIndex();

    GroovesharkTreeItem *parentItem;

    if ( !parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = static_cast<GroovesharkTreeItem*> ( parent.internalPointer() );

    GroovesharkTreeItem *childItem = parentItem->child ( row );
    if ( childItem )
        return createIndex ( row, column, childItem );
    else
        return QModelIndex();
}

QModelIndex GroovesharkTreeModel::parent ( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    GroovesharkTreeItem *childItem = static_cast<GroovesharkTreeItem*> ( index.internalPointer() );
    GroovesharkTreeItem *parentItem = childItem->parent();

    if ( parentItem == m_rootItem )
        return QModelIndex();

    return createIndex ( parentItem->row(), 0, parentItem );
}

int GroovesharkTreeModel::rowCount ( const QModelIndex &parent ) const
{
    GroovesharkTreeItem *parentItem;
    if ( parent.column() > 0 )
        return 0;

    if ( !parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = static_cast<GroovesharkTreeItem*> ( parent.internalPointer() );

    return parentItem->childCount();
}

void GroovesharkTreeModel::setupModelData ( GroovesharkTreeItem *parent )
{
    QList<GroovesharkTreeItem*> parents;
    QList<int> indentations;
    parents << parent;

    parents.last()->appendChild ( new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::MyRecommendations ), Grooveshark::MyRecommendations, parents.last() ) );
    parents.last()->appendChild ( new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::PersonalRadio ), Grooveshark::PersonalRadio, parents.last() ) );
    parents.last()->appendChild ( new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::MixRadio ), Grooveshark::MixRadio, parents.last() ) );
    parents.last()->appendChild ( new GroovesharkTreeItem ( mapTypeToUrl ( Grooveshark::NeighborhoodRadio ), Grooveshark::NeighborhoodRadio, parents.last() ) );

    m_myTopArtists = new GroovesharkTreeItem ( Grooveshark::TopArtists, parents.last() );
    parents.last()->appendChild ( m_myTopArtists );

    m_myTags = new GroovesharkTreeItem ( Grooveshark::MyTags, parents.last() );
    parents.last()->appendChild ( m_myTags );

    m_myFriends = new GroovesharkTreeItem ( Grooveshark::Friends, parents.last() );
    parents.last()->appendChild ( m_myFriends );

    m_myNeighbors = new GroovesharkTreeItem ( Grooveshark::Neighbors, parents.last() );
    parents.last()->appendChild ( m_myNeighbors );


}

QString GroovesharkTreeModel::mapTypeToUrl ( Grooveshark::Type type, const QString &key )
{
    QString const encoded_username = KUrl::toPercentEncoding ( m_userName );
    switch ( type )
    {
    case MyRecommendations:
        return "grooveshark://user/" + encoded_username + "/recommended";
    case PersonalRadio:
        return "grooveshark://user/" + encoded_username + "/personal";
    case MixRadio:
        return "grooveshark://user/" + encoded_username + "/mix";
    case NeighborhoodRadio:
        return "grooveshark://user/" + encoded_username + "/neighbours";
    case MyTagsChild:
        return "grooveshark://usertags/" + encoded_username + "/" + KUrl::toPercentEncoding ( key );
    case FriendsChild:
        return "grooveshark://user/" + KUrl::toPercentEncoding ( key ) + "/personal";
    case ArtistsChild:
        return "grooveshark://artist/" + KUrl::toPercentEncoding ( key ) + "/similarartists";
    case NeighborsChild:
        return "grooveshark://user/" + KUrl::toPercentEncoding ( key ) + "/personal";
    case UserChildPersonal:
        return "grooveshark://user/" + KUrl::toPercentEncoding ( key ) + "/personal";
    case UserChildNeighborhood:
        return "grooveshark://user/" + KUrl::toPercentEncoding ( key ) + "/neighbours";
    default:
        return "";
    }
}

GroovesharkTreeItem::GroovesharkTreeItem ( const Grooveshark::Type &type, const QVariant &data, GroovesharkTreeItem *parent )
        : mType ( type ), parentItem ( parent ), itemData ( data )
{
}

GroovesharkTreeItem::GroovesharkTreeItem ( const Grooveshark::Type &type, GroovesharkTreeItem *parent )
        : mType ( type ), parentItem ( parent )
{

}

GroovesharkTreeItem::GroovesharkTreeItem ( const QString &url, const Grooveshark::Type &type, GroovesharkTreeItem *parent )
        : mType ( type ), parentItem ( parent ), mUrl ( url )
{

}

GroovesharkTreeItem::GroovesharkTreeItem ( const QString &url, const Grooveshark::Type &type, const QVariant &data, GroovesharkTreeItem *parent )
        : mType ( type ), parentItem ( parent ), itemData ( data ), mUrl ( url )
{
}

GroovesharkTreeItem::~GroovesharkTreeItem()
{
    qDeleteAll ( childItems );
}

void GroovesharkTreeItem::appendChild ( GroovesharkTreeItem *item )
{
    childItems.append ( item );
}

GroovesharkTreeItem *GroovesharkTreeItem::child ( int row )
{
    return childItems.value ( row );
}

int GroovesharkTreeItem::childCount() const
{
    return childItems.count();
}

int GroovesharkTreeItem::columnCount() const
{
    return 1;
}

QVariant GroovesharkTreeItem::data () const
{
    return itemData;
}
Meta::TrackPtr GroovesharkTreeItem::track() const
{
    Meta::TrackPtr track;
    if ( mUrl.isEmpty() )
        return track;

    KUrl url ( mUrl );
    track = CollectionManager::instance()->trackForUrl ( url );

    return track;
}

GroovesharkTreeItem *GroovesharkTreeItem::parent()
{
    return parentItem;
}

int GroovesharkTreeItem::row() const
{
    if ( parentItem )
        return parentItem->childItems.indexOf ( const_cast<GroovesharkTreeItem*> ( this ) );

    return 0;
}

QMimeData*
GroovesharkTreeModel::mimeData( const QModelIndexList &indices ) const
{
    debug() << "Grooveshark drag items : " << indices.size();
    Meta::TrackList list;
    foreach ( const QModelIndex &item, indices )
    {
        Meta::TrackPtr track = data( item, Grooveshark::TrackRole ).value< Meta::TrackPtr >();
        if ( track )
            list << track;
    }
    qStableSort ( list.begin(), list.end(), Meta::Track::lessThan );

    AmarokMimeData *mimeData = new AmarokMimeData();
    mimeData->setTracks( list );
    return mimeData;
}
