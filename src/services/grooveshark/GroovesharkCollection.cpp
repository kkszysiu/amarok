/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#include "GroovesharkCollection.h"

#include "core/support/Debug.h"
#include "GroovesharkCollectionLocation.h"
#include "GroovesharkMeta.h"

#include "ServiceMetaBase.h"

#include "core-impl/collections/support/MemoryQueryMaker.h"

using namespace Collections;

GroovesharkCollection::GroovesharkCollection( const QString & prettyName )
: ServiceCollection( 0, "last.fm", "last.fm" )
//    : ServiceSqlCollection( id, prettyName, metaFactory, registry )
{
    debug() << "GroovesharkCollection::GroovesharkCollection:" << prettyName;
}

Meta::TrackPtr GroovesharkCollection::trackForUrl(const KUrl & url)
{
    DEBUG_BLOCK
    debug() << "Meta::TrackPtr GroovesharkCollection::trackForUrl:" << url;

    QString pristineUrl = url.url();

    if ( pristineUrl.startsWith( "http://grooveshark.com/playlist_redirect.php?url=" ) ) {

        //if we are not a member of the right type, we need to preserve this or we will not be able to play the track. Actually... use the original url in any case so plays are attributed to the person whose playlist it is...
        QString orgUrl = pristineUrl;

        int endIndex = pristineUrl.indexOf( "&key=" );

        pristineUrl = pristineUrl.mid( 47, endIndex - 47 );

        //debug() << "got redirected url: " << pristineUrl;


        pristineUrl.remove( "_nospeech" );
        pristineUrl.replace( ".ogg", ".mp3" );
        pristineUrl.replace( "-lofi.mp3", ".mp3" );

        pristineUrl.replace( QRegExp( "http://download" ), "http://he3" );
        pristineUrl.replace( QRegExp( "http://stream" ), "http://he3" );

        //debug() << "after a quick makeover: " << pristineUrl;

        Meta::TrackPtr trackPtr = ServiceCollection::trackForUrl( KUrl( pristineUrl ) );

        if ( trackPtr ) {
            Meta::ServiceTrack * mTrack = dynamic_cast< Meta::ServiceTrack * >( trackPtr.data() );
            if ( mTrack ) {

                mTrack->setUidUrl( orgUrl );
            }
        }

        return trackPtr;

    } else {

        pristineUrl.remove( "_nospeech" );
        pristineUrl.replace( ".ogg", ".mp3" );
        pristineUrl.replace( "-lofi.mp3", ".mp3" );

        pristineUrl.replace( QRegExp( ".*:.*@download" ), "http://he3" );
        pristineUrl.replace( QRegExp( ".*:.*@stream" ), "http://he3" );

        return ServiceCollection::trackForUrl( KUrl( pristineUrl ) );

    }
    
}

CollectionLocation * GroovesharkCollection::location() const
{
    return new GroovesharkCollectionLocation( this );
}




