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
 
#ifndef GROOVESHARKCOLLECTIONLOCATION_H
#define GROOVESHARKCOLLECTIONLOCATION_H

#include "GroovesharkSqlCollection.h"
#include "ServiceCollectionLocation.h"

namespace Collections {

/**
A ServiceCollectionLocation subclass responsible for showing a small Grooveshark specific dialog when copying tracks from Grooveshark

	@author Nikolaj Hald Nielsen <nhn@kde.org>
*/
class GroovesharkCollectionLocation : public ServiceCollectionLocation
{
public:
    GroovesharkCollectionLocation( GroovesharkSqlCollection const *parentCollection );

    virtual ~GroovesharkCollectionLocation();

    virtual void showSourceDialog( const Meta::TrackList &tracks, bool removeSources );

};

} //namespace Collections

#endif
