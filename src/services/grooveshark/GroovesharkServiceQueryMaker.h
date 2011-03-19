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

#ifndef GROOVESHARKSERVICEQUERYMAKER_H
#define GROOVESHARKSERVICEQUERYMAKER_H

#include "DynamicServiceQueryMaker.h"

namespace Collections {

class GroovesharkServiceCollection;

class GroovesharkServiceQueryMaker : public DynamicServiceQueryMaker
{
    Q_OBJECT

public:
    GroovesharkServiceQueryMaker( GroovesharkServiceCollection *collection );
    virtual ~GroovesharkServiceQueryMaker();

    virtual void run();
    virtual void abortQuery();

    virtual QueryMaker *setReturnResultAsDataPtrs( bool resultAsDataPtrs );

private:
    bool m_resultAsDataPtrs;
};

} //namespace Collections

#endif // GROOVESHARKSERVICEQUERYMAKER_H
