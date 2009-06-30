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
 
#ifndef MAINTOOLBARNG_H
#define MAINTOOLBARNG_H

#include "CurrentTrackToolbar.h"

#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

/**
An new toolbar implementation.

	@author 
*/
class MainToolbarNG : public QToolBar
{
    Q_OBJECT
public:
    MainToolbarNG( QWidget * parent );

    ~MainToolbarNG();

private slots:
    void engineVolumeChanged( int newVolume );
    void engineMuteStateChanged( bool muted );
    
private:

    CurrentTrackToolbar * m_currentTrackToolbar;
    
    QToolButton * m_volumeToolButton;
    QLabel * m_volumeLabel; 
    QMenu * m_volumeMenu;

};

#endif