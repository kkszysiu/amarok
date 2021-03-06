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
 
#ifndef GROOVESHARKACTIONS_H
#define GROOVESHARKACTIONS_H

#include "GroovesharkMeta.h"

#include <QAction>

/**
A simple QAction subclass for purchasing or downloading content from Grooveshark

	@author Nikolaj Hald Nielsen <nhn@kde.org>
*/
class GroovesharkDownloadAction : public QAction
{
    Q_OBJECT
public:
    GroovesharkDownloadAction( const QString &text, Meta::GroovesharkAlbum * album );

    ~GroovesharkDownloadAction();

private slots:
    void slotTriggered();

private:
    Meta::GroovesharkAlbum * m_album;

};

/**
A simple QAction subclass for purchasing or downloading content from Grooveshark

    @author Nikolaj Hald Nielsen <nhn@kde.org>
*/
class GroovesharkAddToFavoritesAction : public QAction
{
    Q_OBJECT
public:
    GroovesharkAddToFavoritesAction( const QString &text, Meta::GroovesharkAlbum * album );

    ~GroovesharkAddToFavoritesAction();

private slots:
    void slotTriggered();

private:
    Meta::GroovesharkAlbum * m_album;

};

#endif
