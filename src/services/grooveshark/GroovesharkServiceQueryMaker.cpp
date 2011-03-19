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

#include "GroovesharkServiceQueryMaker.h"

using namespace Collections;

GroovesharkServiceQueryMaker::GroovesharkServiceQueryMaker( GroovesharkServiceCollection *collection )
    : m_resultAsDataPtrs( false )
{
    Q_UNUSED( collection );
}


GroovesharkServiceQueryMaker::~GroovesharkServiceQueryMaker()
{
}

void 
GroovesharkServiceQueryMaker::run()
{
}
    

void 
GroovesharkServiceQueryMaker::abortQuery()
{
}


QueryMaker*
GroovesharkServiceQueryMaker::setReturnResultAsDataPtrs( bool resultAsDataPtrs )
{
    m_resultAsDataPtrs = resultAsDataPtrs;
    return this;
}
