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

#ifndef GROOVESHARKTREEITEM_H_
#define GROOVESHARKTREEITEM_H_

#include <QVariant>
#include <QList>
#include <QStringList>
#include <QModelIndex>

class GroovesharkServiceModel;

class GroovesharkTreeItem : public QObject
{
    Q_OBJECT
public:
    GroovesharkTreeItem( GroovesharkTreeItem *parent = 0 );
    virtual ~GroovesharkTreeItem();

    void appendChild( GroovesharkTreeItem *child );

    GroovesharkTreeItem *child( int row );
    int childCount() const;
    void setHasChildren( bool hasChildren );
    bool hasChildren() const;

    GroovesharkTreeItem *parent() const;
    bool isRoot() const;


    virtual QVariant displayData() const;

    virtual void appendTags( QStringList tags );
    virtual void appendPodcasts( QStringList podcasts );
private:
    QList<GroovesharkTreeItem*> m_childItems;
    GroovesharkTreeItem *m_parentItem;
    bool m_hasChildren;
};

#endif /* GROOVESHARKTREEITEM_H_ */
