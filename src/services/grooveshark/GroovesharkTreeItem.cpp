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

#include "GroovesharkTreeItem.h"
#include "GroovesharkServiceModel.h"
#include "GroovesharkPodcastTreeItem.h"
#include "GroovesharkTagTreeItem.h"

#include <QDebug>

GroovesharkTreeItem::GroovesharkTreeItem( GroovesharkTreeItem *parent ) : QObject( parent ), m_parentItem( parent ), m_hasChildren( false )
{

}

GroovesharkTreeItem::~GroovesharkTreeItem()
{
    qDeleteAll( m_childItems );
}

void GroovesharkTreeItem::appendChild( GroovesharkTreeItem *item )
{
    m_childItems.append( item );
}

GroovesharkTreeItem *GroovesharkTreeItem::child( int row )
{
    return m_childItems.value( row );
}

bool GroovesharkTreeItem::hasChildren() const
{
    return m_hasChildren;
}

void GroovesharkTreeItem::setHasChildren( bool hasChildren )
{
    m_hasChildren = hasChildren;
}

int GroovesharkTreeItem::childCount() const
{
    return m_childItems.count();
}

GroovesharkTreeItem *GroovesharkTreeItem::parent() const
{
    return m_parentItem;
}

QVariant GroovesharkTreeItem::displayData() const
{
    return QVariant();
}

bool GroovesharkTreeItem::isRoot() const
{
    return ( m_parentItem == 0 );
}

void GroovesharkTreeItem::appendTags( QStringList tags )
{
    QString str;
    foreach( str, tags )
    {
        GroovesharkTagTreeItem *treeItem = new GroovesharkTagTreeItem( str, this );
        appendChild( treeItem );
    }
}

void GroovesharkTreeItem::appendPodcasts( QStringList podcasts )
{
    foreach( QString podcast, podcasts )
    {
        appendChild( new GroovesharkPodcastTreeItem( podcast, this ) );
    }
}
