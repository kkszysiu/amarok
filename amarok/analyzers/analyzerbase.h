/***************************************************************************
                               analyzerbase.h
                             -------------------
    begin                : Die Jan 7 2003
    copyright            : (C) 2003 by Mark Kretschmann
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ANALYZERBASE_H
#define ANALYZERBASE_H

#ifdef __FreeBSD__
    #include <sys/types.h>
#endif

#include <config.h>  //HAVE_QGLWIDGET
#include "fht.h"     //stack allocated and convenience
#include <qpixmap.h> //stack allocated and convenience
#include <qtimer.h>  //stack allocated
#include <qwidget.h> //baseclass
#include <vector>    //included for convenience

#ifdef HAVE_QGLWIDGET
#include <qgl.h>     //baseclass
#include <GL/gl.h>  //included for convenience
#include <GL/glu.h> //included for convenience
typedef QGLWidget DUMMY;
#else
typedef QWidget DUMMY;
#endif

class QEvent;
class QPaintEvent;


namespace Analyzer {

typedef std::vector<float> Scope;

//TODO what would be grand would be to get the raw scope by reference so we could hang on to it for the purpose of paused()

template<class W> class Base : public W
{
public:
    uint timeout() const { return m_timeout; }
    uint height()  const { return m_height; }

protected:
    Base( QWidget*, uint, uint = 7 );

    void drawFrame();
    virtual void transform( Scope& );
    virtual void analyze( const Scope& ) = 0;
    virtual void paused();
    virtual void demo();

private:
    bool event( QEvent* );

protected:
    QTimer m_timer;
    uint   m_timeout;
    uint   m_height;
    FHT    m_fht;
};


class Base2D : public Base<QWidget>
{
Q_OBJECT
public:
    const QPixmap *background() const { return &m_background; }
    const QPixmap *canvas()     const { return &m_canvas; }

private slots:
    void draw() { drawFrame(); bitBlt( this, 0, 0, canvas() ); }

protected:
    Base2D( QWidget*, uint, uint = 7 );

    virtual void init() {}

    QPixmap  *canvas() { return &m_canvas; }
    void eraseCanvas() { bitBlt( canvas(), 0, 0, background() ); }

private:
    void polish();
    void paintEvent( QPaintEvent* ) { if( !m_canvas.isNull() ) bitBlt( this, 0, 0, canvas() ); }

    QPixmap m_background;
    QPixmap m_canvas;
};



//this mess is because moc generates an entry for this class despite the #if block
//1. the Q_OBJECT macro must be exposed
//2. we have to define the class
//3. we have to declare a ctor (to satisfy the inheritance)
//4. the slot must also by visible (!)
//5. the DUMMY may not be necessary provided you include <qgl.h>, but I wasn't certain if this would cause
//   link failure, so it's there to be sure.
//TODO find out how to stop moc generating a metaobject for this class
class Base3D : public Base<DUMMY>
{
Q_OBJECT
private slots:
    void draw() { drawFrame(); }
#ifdef HAVE_QGLWIDGET
protected:
    Base3D( QWidget*, uint, uint = 7 );
#else
protected:
    Base3D( QWidget *w, uint i1, uint i2 ) : Base<DUMMY>( w, i1, i2 ) {}
#endif
};


class Factory
{
    //Currently this is a rather small class, its only purpose
    //to ensure that making changes to analyzers will not require
    //rebuilding the world!

    //eventually it would be better to make analyzers pluggable
    //but I can't be arsed, nor can I see much reason to do so
    //yet!
public:
    static QWidget* createAnalyzer( QWidget* );
};


void interpolate( const Scope&, Scope& );
void initSin( Scope&, const uint = 6000 );

} //END namespace Analyzer

using Analyzer::Scope;

#endif
