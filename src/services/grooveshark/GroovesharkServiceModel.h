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

#ifndef GROOVESHARKSERVICEMODEL_H_
#define GROOVESHARKSERVICEMODEL_H_

#include <QAbstractItemModel>
#include <QStringList>
//#include "NetworkAccessManagerProxy.h"
#include <QNetworkReply>

#include "GroovesharkTreeItem.h"

#include "grooveshark/QGroovesharkSong.h"
#include "grooveshark/QGrooveshark.h"

class GroovesharkTreeItem;


class GroovesharkServiceModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit GroovesharkServiceModel( QGrooveshark &manager, QObject *parent = 0 );
    virtual ~GroovesharkServiceModel();

    // QAbstractItemModel methods
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;
//    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    void insertPodcastList( QStringList podcasts, const QModelIndex &parentItem, bool isMain = false );

private slots:
    void topTagsRequestError( QNetworkReply::NetworkError error );
    void topTagsParseError();
    void insertTagList();

    void onSearchResultsReceived(QList<QGroovesharkSong*> songlist);

protected:
    virtual bool canFetchMore( const QModelIndex &parent ) const;
    virtual void fetchMore( const QModelIndex &parent );

private:
    GroovesharkTreeItem *rootItem;

};

#endif /* GROOVESHARKSERVICEMODEL_H_ */
