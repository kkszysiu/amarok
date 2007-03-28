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

#include "metaquerybuilder.h"

#include <QGlobal>

MetaQueryBuilder::MetaQueryBuilder( const QList<Collection*> &collections )
    : super()
    , m_queryDoneCount( 0 )
    , m_queryDoneCountMutex()
{
    foreach( Collection *c, collections )
    {
        QueryBuilder *b = c->queryBuilder();
        builders.append( b );
        connect( b, SIGNAL( queryDone() ), this, SLOT( slotQueryDone() ) );
    }
}

MetaQueryBuilder::~MetaQueryBuilder()
{
    foreach( QueryBuilder *b, collections )
        delete b;
}

QueryBuilder*
MetaQueryBuilder::reset()
{
    m_queryDoneCount = 0;
    foreach( QueryBuilder *b, collections )
        b->reset();
    return this;
}

QueryBuilder*
MetaQueryBuilder::run()
{
    foreach( QueryBuilder *b, collections )
        b->run();
    return this;
}

QueryBuilder*
MetaQueryBuilder::abortQuery()
{
    foreach( QueryBuilder *b, collections )
        b->abortQuery();
    return this;
}

QueryBuilder*
MetaQueryBuilder::startTrackQuery()
{
    foreach( QueryBuilder *b, collections )
        b->startTrackQuery();
    return this;
}

QueryBuilder*
MetaQueryBuilder::startArtistQuery()
{
    foreach( QueryBuilder *b, collections )
        b->startArtistQuery();
    return this;
}

QueryBuilder*
MetaQueryBuilder::startAlbumQuery()
{
    foreach( QueryBuilder *b, collections )
        b->startAlbumQuery();
    return this;
}

QueryBuilder*
MetaQueryBuilder::startGenreQuery()
{
    foreach( QueryBuilder *b, collections )
        b->startGenreQuery();
    return this;
}

QueryBuilder*
MetaQueryBuilder::startComposerQuery()
{
    foreach( QueryBuilder *b, collections )
        b->startComposerQuery();
    return this;
}

QueryBuilder*
MetaQueryBuilder::startYearQuery()
{
    foreach( QueryBuilder *b, collections )
        b->startYearQuery();
    return this;
}

QueryBuilder*
MetaQueryBuilder::addReturnValue()
{
    foreach( QueryBuilder *b, collections )
        b->addReturnValue();
    return this;
}

QueryBuilder*
MetaQueryBuilder::orderBy()
{
    foreach( QueryBuilder *b, collections )
        b->orderBy();
    return this;
}

QueryBuilder*
MetaQueryBuilder::includeCollection( const QString &collectionId )
{
    foreach( QueryBuilder *b, collections )
        b->includeCollection( collectionId );
    return this;
}

QueryBuilder*
MetaQueryBuilder::excludeCollection( const QString &collectionId )
{
    foreach( QueryBuilder *b, collections )
        b->excludeCollection( collectionid );
    return this;
}

void
MetaQueryBuilder::slotQueryDone()
{
    m_queryDoneCountMutex.lock();
    m_queryDoneCount++;
    if ( m_queryDoneCount == builders.size() )
    {
        //make sure we don't give control to code outside this class while holding the lock
        m_queryDoneCountMutex.unlock();
        emit queryDone();
    }
    else
        m_queryDoneCountMutex.unlock();
}

#include "metaquerybuilder.moc"
