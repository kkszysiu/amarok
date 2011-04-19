/****************************************************************************************
 * Copyright (c) 2011 Stefan Derkits <stefan@derkits.at>                                *
 * Copyright (c) 2011 Christian Wagner <christian.wagner86@gmx.at>                      *
 * Copyright (c) 2011 Felix Winter <ixos01@gmail.com>                                   *
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

#include "GroovesharkServiceModel.h"
#include "GroovesharkTagTreeItem.h"
#include "GroovesharkPodcastTreeItem.h"
#include <QList>
#include <QEventLoop>

#include "core/support/Debug.h"

static const int s_numberItemsToLoad = 100;

GroovesharkServiceModel::GroovesharkServiceModel( QGrooveshark &manager, QObject *parent ) : QAbstractItemModel( parent )
{
    rootItem = new GroovesharkTreeItem();
    connect(&manager, SIGNAL(onSearchResultsReceived(QList<QGroovesharkSong*>)), this, SLOT(onSearchResultsReceived(QList<QGroovesharkSong*>)));
}

GroovesharkServiceModel::~GroovesharkServiceModel()
{
    delete rootItem;
}

//Qt::ItemFlags GroovesharkServiceModel::flags( const QModelIndex &idx ) const
//{
//	DEBUG_BLOCK
//    if( !idx.isValid() )
//        return Qt::ItemIsDropEnabled;
//
//    GroovesharkTreeItem *treeItem = static_cast<GroovesharkTreeItem *>( idx.internalPointer() );
//    if( treeItem ) //probably a folder
//        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
//                | Qt::ItemIsDropEnabled;
//
//    return QAbstractItemModel::flags( idx );
//}


QModelIndex GroovesharkServiceModel::index( int row, int column, const QModelIndex &parent ) const
{
    DEBUG_BLOCK

    if( !hasIndex( row, column, parent ) )
        return QModelIndex();

    GroovesharkTreeItem *parentItem;

    if( !parent.isValid() )
        parentItem = rootItem;
    else
        parentItem = static_cast<GroovesharkTreeItem*>( parent.internalPointer() );

    if( parentItem == 0 )
        return QModelIndex();

    GroovesharkTreeItem *childItem = parentItem->child( row );
    if( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

QModelIndex GroovesharkServiceModel::parent( const QModelIndex &index ) const
{
    DEBUG_BLOCK

    if( !index.isValid() )
        return QModelIndex();

    GroovesharkTreeItem *childItem = static_cast<GroovesharkTreeItem*>( index.internalPointer() );

    if( childItem == 0 || childItem->isRoot() )
        return QModelIndex();

    GroovesharkTreeItem *parentItem = childItem->parent();

    if( parentItem == 0 )
    {
        return QModelIndex();
    }

    int childIndex;
    if( parentItem->isRoot() )
        return QModelIndex();
    else
        childIndex = parentItem->parent()->children().indexOf( parentItem );

    return createIndex( childIndex, 0, parentItem );
}

int GroovesharkServiceModel::rowCount( const QModelIndex &parent ) const
{
    DEBUG_BLOCK

    GroovesharkTreeItem *parentItem;

    if( !parent.isValid() )
    {
        return rootItem->childCount();
    }

    parentItem = static_cast<GroovesharkTreeItem*>( parent.internalPointer() );

    if( parentItem == 0 )
        return 0;

    return parentItem->childCount();
}

int GroovesharkServiceModel::columnCount( const QModelIndex &parent ) const
{
    DEBUG_BLOCK

    Q_UNUSED( parent )
    return 1;
}

QVariant GroovesharkServiceModel::data( const QModelIndex &index, int role ) const
{
    DEBUG_BLOCK

    if( !index.isValid() )
        return QVariant();

    if( role != Qt::DisplayRole )
        return QVariant();

    GroovesharkTreeItem *item = static_cast<GroovesharkTreeItem*>( index.internalPointer() );
    if( item == 0 )
    {
        return QVariant();
    }

    return item->displayData();
}

void GroovesharkServiceModel::insertTagList()
{
    DEBUG_BLOCK

    qDebug() << "GroovesharkServiceModel::insertTagList()";

    QStringList topTags;
    topTags << "Arial" << "Helvetica" << "Times" << "Courier";

    if( rootItem != 0 )
    {
        beginInsertRows( QModelIndex(), 0, topTags.count() - 1 );
        rootItem->appendTags( topTags );
        endInsertRows();
    }
}

void GroovesharkServiceModel::topTagsRequestError( QNetworkReply::NetworkError error )
{
    debug() << "Error in TopTags request: " << error;
}

void GroovesharkServiceModel::topTagsParseError()
{
    debug() << "Error while parsing TopTags";
}

void GroovesharkServiceModel::insertPodcastList( QStringList podcasts, const QModelIndex & parentItem, bool isMain )
{
    DEBUG_BLOCK

    emit layoutAboutToBeChanged();
    if (!isMain) {
        beginInsertRows( parentItem, 0, podcasts.count() - 1 );
        GroovesharkTreeItem *item = static_cast<GroovesharkTreeItem*>( parentItem.internalPointer() );
        if( item != 0 )
        {
            debug() << "Appending Podcasts...";
            item->appendPodcasts( podcasts );
        }
        endInsertRows();
    } else {
        if( rootItem != 0 )
        {
            beginInsertRows( QModelIndex(), 0, podcasts.count() - 1 );
            rootItem->appendPodcasts( podcasts );
            endInsertRows();
        }
    }

    emit layoutChanged();
}

bool GroovesharkServiceModel::hasChildren( const QModelIndex &parent ) const
{
    DEBUG_BLOCK

    if( !parent.isValid() )
        return true;

    GroovesharkTreeItem *treeItem = static_cast<GroovesharkTreeItem *>( parent.internalPointer() );

    if( treeItem == 0 )
        return false;

    if( treeItem->childCount() > 0 )
        return true;

    if( !qobject_cast<GroovesharkPodcastTreeItem *>( treeItem ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool GroovesharkServiceModel::canFetchMore( const QModelIndex &parent ) const
{
    DEBUG_BLOCK

    // root item
    if( !parent.isValid() )
    {
        return !rootItem->hasChildren();
    }

    // already fetched or just started?
    GroovesharkTreeItem *treeItem = static_cast<GroovesharkTreeItem *>( parent.internalPointer() );
    if( treeItem == 0 || treeItem->hasChildren() /* || m_currentFetchingMap.values().contains( parent ) */ )
    {
        return false;
    }

    // TagTreeItem

    if( qobject_cast<GroovesharkTagTreeItem*>( treeItem ) )
    {
        return true;
    }
    return false;
}

void GroovesharkServiceModel::fetchMore( const QModelIndex &parent )
{
    DEBUG_BLOCK

    // root item
    if( !parent.isValid() )
    {
/*
        topTags = m_request.topTags( s_numberItemsToLoad );
        rootItem->setHasChildren( true );
        connect( topTags.data(), SIGNAL( finished() ), this, SLOT( insertTagList() ) );
        connect( topTags.data(), SIGNAL( requestError( QNetworkReply::NetworkError ) ), SLOT( topTagsRequestError( QNetworkReply::NetworkError ) ) );
        connect( topTags.data(), SIGNAL( parseError() ), SLOT( topTagsParseError() ) );
*/
        insertTagList();
        rootItem->setHasChildren( true );
    }

    // TagTreeItem
    GroovesharkTreeItem *treeItem = static_cast<GroovesharkTreeItem *>( parent.internalPointer() );

    if( GroovesharkTagTreeItem *tagTreeItem = qobject_cast<GroovesharkTagTreeItem*>( treeItem ) )
    {
        tagTreeItem->setHasChildren( true );

//         QStringList *podcastsTest = new QStringList();
//         podcastsTest->append("Test1");
//         podcastsTest->append("Test2");
//         podcastsTest->append("Test3");
//         podcastsTest->append("Test4");
//         podcastsTest->append("Test5");
        
        
        //insertPodcastList(podcastsTest, parent);
        //mygpo::PodcastListPtr podcasts = m_request.podcastsOfTag( s_numberItemsToLoad, tagTreeItem->tag()->tag() );
        //GpodderPodcastRequestHandler *podcastRequestHandler = new GpodderPodcastRequestHandler( podcasts, parent, this );
        //connect( podcasts.data(), SIGNAL( finished() ), podcastRequestHandler, SLOT( finished() ) );
        //connect( podcasts.data(), SIGNAL( requestError( QNetworkReply::NetworkError ) ), podcastRequestHandler, SLOT( topTagsRequestError( QNetworkReply::NetworkError ) ) );
        //connect( podcasts.data(), SIGNAL( parseError() ), podcastRequestHandler, SLOT( topTagsParseError() ) );
    }
}


void GroovesharkServiceModel::onSearchResultsReceived(QList<QGroovesharkSong*> songlist) {
    //m_searchWidget->setVisible(true);
    
    debug() << "songlist:" << songlist;
    qDebug() << "songlist:" << songlist;
    // TODO: do some cool stuff with songs, list them in the treeview :)

    QStringList podcastsTest;
    podcastsTest << "Test1" << "Test2" << "Test3" << "Test4";

    insertPodcastList(podcastsTest, QModelIndex(), true);
}



