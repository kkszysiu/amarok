/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// Maintainer: Max Howell <max.howell@methylblue.com>
// Authors:    Mark Kretschmann & Max Howell (C) 2003-4
//

#ifndef BARANALYZER_H
#define BARANALYZER_H

#include "analyzerbase.h"
//Added by qt3to4:
#include <QResizeEvent>
#include <QPalette>
#include <QPixmap>

typedef std::vector<uint> aroofMemVec;


class BarAnalyzer : public Analyzer::Base2D
{
    public:
        BarAnalyzer( QWidget* );

        void init();
        virtual void analyze( const Scope& );
        //virtual void transform( Scope& );

        /**
         * Resizes the widget to a new geometry according to @p e
         * @param e The resize-event
         */
        void resizeEvent( QResizeEvent * e);

        uint BAND_COUNT;
        int MAX_DOWN;
        int MAX_UP;
        static const uint ROOF_HOLD_TIME = 48;
        static const int  ROOF_VELOCITY_REDUCTION_FACTOR = 32;
        static const uint NUM_ROOFS = 16;
        static const uint COLUMN_WIDTH = 4;

    protected:
        QPixmap m_pixRoof[NUM_ROOFS];
        //vector<uint> m_roofMem[BAND_COUNT];
        virtual void paintEvent( QPaintEvent* );

        Scope m_bands; //copy of the Scope to prevent creating/destroying a Scope every iteration
        uint  m_lvlMapper[256];
        std::vector<aroofMemVec> m_roofMem;
        std::vector<uint> barVector;          //positions of bars
        std::vector<int>  roofVector;         //positions of roofs
        std::vector<uint> roofVelocityVector; //speed that roofs falls

        const QPixmap *gradient() const { return &m_pixBarGradient; }

    private:
        QPixmap m_pixBarGradient;
        Scope m_scope;             //so we don't create a vector every frame
        QPalette::ColorRole m_bg;
};

#endif
