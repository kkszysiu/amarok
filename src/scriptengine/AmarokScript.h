/******************************************************************************
 * Copyright (C) 2008 Peter ZHOU <peterzhoulei@gmail.com>                     *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#ifndef AMAROK_SCRIPT_H
#define AMAROK_SCRIPT_H

#include <QObject>

class QScriptEngine;

namespace AmarokScript
{

    class AmarokScript : public QObject
    {
        Q_OBJECT

        public:
            AmarokScript( QScriptEngine* scriptEngine );
            ~AmarokScript();
            void slotConfigured();

        public slots:
            void        quit() const;
            int         alert( const QString& text, const QString& type = "information" ) const;
            bool        runScript( const QString& name ) const;
            bool        stopScript( const QString& name ) const;
            QStringList listRunningScripts() const;
        signals:
            void configured();

        private slots:

    };
}

#endif
