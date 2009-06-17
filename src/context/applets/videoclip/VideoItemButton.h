/***************************************************************************
* copyright   : (C) 2009 Simon Esneault <simon.esneault@gmail.com>        *
**************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef VIDEOITEMBUTTON_H
#define VIDEOITEMBUTTON_H

#include "VideoclipApplet.h"

#include <QToolButton>


/** VideoItemButton specialized widget for interaction
* - middle click will queue the track
* - double click will append and play the track
* - right click opens a popup menu with options.
*/

class VideoItemButton : public QToolButton
{
public:
    Q_OBJECT
    
public:
    VideoItemButton( QWidget* parent = 0 );
    
    void setVideoInfo( VideoInfo * );
    VideoInfo * getVideoInfo();

protected:
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseDoubleClickEvent( QMouseEvent* );
        
    
public slots:
    void append();
    void queue();
    void appendPlay();
    void myMenu( QPoint point);
    
signals:
    void appendRequested( VideoInfo * );
    void appendPlayRequested( VideoInfo * );
    void queueRequested( VideoInfo * );
    
private :
    VideoInfo *m_videoInfo;
};

#endif // VIDEOITEMBUTTON_H