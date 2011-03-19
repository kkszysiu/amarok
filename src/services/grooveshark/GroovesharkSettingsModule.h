/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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
 
#ifndef GROOVESHARKSETTINGSMODULE_H
#define GROOVESHARKSETTINGSMODULE_H

#include "GroovesharkConfig.h"

#include <kcmodule.h>

namespace Ui { class GroovesharkConfigWidget; }

/**
A KCM module to configure the Grooveshark service

	@author 
*/
class GroovesharkSettingsModule : public KCModule
{
    Q_OBJECT
public:
    explicit GroovesharkSettingsModule( QWidget *parent = 0, const QVariantList &args = QVariantList() );

    ~GroovesharkSettingsModule();

    virtual void save();
    virtual void load();
    virtual void defaults();

private slots:

    void settingsChanged();

private:

    GroovesharkConfig m_config;
    Ui::GroovesharkConfigWidget * m_configDialog;

};

#endif
