/***************************************************************************
 *   Copyright (c) 2009  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
 
#include "CurrentTrackToolbar.h"

#include "EngineController.h"
#include "GlobalCurrentTrackActions.h"
#include "meta/capabilities/CurrentTrackActionsCapability.h"

CurrentTrackToolbar::CurrentTrackToolbar( QWidget * parent )
 : QToolBar( parent )
 , EngineObserver( The::engineController() ) 
{
    setToolButtonStyle( Qt::ToolButtonIconOnly );
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    //setIconDimensions( 16 );
    setContentsMargins( 0, 0, 0, 0 );
}


CurrentTrackToolbar::~CurrentTrackToolbar()
{
}

void CurrentTrackToolbar::engineStateChanged( Phonon::State state, Phonon::State oldState )
{
    handleAddActions();
}

void CurrentTrackToolbar::engineNewMetaData( const QHash< qint64, QString > & newMetaData, bool trackChanged )
{
}

void CurrentTrackToolbar::handleAddActions()
{
    clear();

    Meta::TrackPtr track = The::engineController()->currentTrack();

    foreach( QAction* action, The::globalCurrentTrackActions()->actions() )
        addAction( action );
    
    if ( track && track->hasCapabilityInterface( Meta::Capability::CurrentTrackActions ) )
    {
        Meta::CurrentTrackActionsCapability *cac = track->create<Meta::CurrentTrackActionsCapability>();
        if( cac )
        {
            QList<PopupDropperAction *> currentTrackActions = cac->customActions();
            foreach( PopupDropperAction *action, currentTrackActions )
                addAction( action );

        }
        delete cac;
    }

}

