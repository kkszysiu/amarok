/****************************************************************************************
 * Copyright (c) 2008 Shane King <kde@dontletsstart.com>                                *
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

#ifndef AMAROK_MULTIPLAYABLECAPABILITYIMPL_P_H
#define AMAROK_MULTIPLAYABLECAPABILITYIMPL_P_H

#include "core/support/Debug.h"
#include "GroovesharkMeta.h"
#include "core/meta/Meta.h"
#include "core/capabilities/MultiPlayableCapability.h"
#include "StatusBar.h"

//#include <grooveshark/Track>
//#include <grooveshark/RadioTuner>
//#include <grooveshark/ws.h>

#include <KLocale>

class MultiPlayableCapabilityImpl : public Capabilities::MultiPlayableCapability, public Meta::Observer
{
    Q_OBJECT
    public:
        MultiPlayableCapabilityImpl( Grooveshark::Track *track )
            : Capabilities::MultiPlayableCapability()
            , m_url( track->internalUrl() )
            , m_track( track )
        {
            //, m_currentTrack( grooveshark::Track() )
            Meta::TrackPtr trackptr( track );
            subscribeTo( trackptr );
            
            connect( track, SIGNAL( skipTrack() ), this, SLOT( skip() ) );
            connect( The::mainWindow(), SIGNAL( skipTrack() ), SLOT( skip() ) );
        }

        virtual ~MultiPlayableCapabilityImpl() 
        {}

        virtual void fetchFirst()
        {
            DEBUG_BLOCK
            /*
            m_tuner = new grooveshark::RadioTuner( grooveshark::RadioStation( m_track->uidUrl() ) );
            
            connect( m_tuner, SIGNAL( trackAvailable() ), this, SLOT( slotNewTrackAvailable() ) );
            connect( m_tuner, SIGNAL( error( grooveshark::ws::Error ) ), this, SLOT( error( grooveshark::ws::Error ) ) );
            */
        }
        
        virtual void fetchNext()
        {
            DEBUG_BLOCK
            //m_currentTrack = m_tuner->takeNextTrack();
            //m_track->setTrackInfo( m_currentTrack );

        }
        
        using Observer::metadataChanged;
        virtual void metadataChanged( Meta::TrackPtr track )
        {
            const Grooveshark::TrackPtr ltrack = Grooveshark::TrackPtr::dynamicCast( track );
            
            if( ltrack.isNull() )
                return;

            KUrl url = ltrack->internalUrl();
            if( url.isEmpty() || url != m_url ) // always should let empty url through, since otherwise we swallow an error getting first track
            {
                m_url = url;
                emit playableUrlFetched( url );
            }
        }

    public slots:

        void slotNewTrackAvailable()
        {
            /*
            if( m_currentTrack.isNull() ) // we only force a track change at the beginning
            {
                m_currentTrack = m_tuner->takeNextTrack();
                m_track->setTrackInfo( m_currentTrack );
            }
            */
        }
        
        virtual void skip()
        {
            fetchNext();
            // now we force a new signal to be emitted to kick the enginecontroller to moving on
            //KUrl url = m_track->playableUrl();
            //emit playableUrlFetched( url );
        }
        
        /*
        void error( grooveshark::ws::Error e )
        {
            if( e == grooveshark::ws::SubscribersOnly || e == grooveshark::ws::AuthenticationFailed )
            {   // Grooveshark is returning an AuthenticationFailed message when the user is not a subscriber, even if the credentials are OK
            The::statusBar()->shortMessage( i18n( "To listen to this stream you need to be a paying Grooveshark subscriber. All the other Grooveshark features are unaffected." ) );
            } else {
                The::statusBar()->shortMessage( i18n( "Error starting track from Grooveshark radio" )   );
            }
        }
        */
        void error( QString ) {
        
        }


    private:
        KUrl m_url;
        Grooveshark::TrackPtr m_track;

        
        //grooveshark::Track m_currentTrack;
        //grooveshark::RadioTuner* m_tuner;
};

#endif
