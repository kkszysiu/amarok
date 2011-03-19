/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
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

#ifndef GROOVESHARKSERVICESETTINGS_H
#define GROOVESHARKSERVICESETTINGS_H

#include "GroovesharkServiceConfig.h"

#include <kcmodule.h>

#include <QNetworkReply>

namespace Ui { class GroovesharkConfigWidget; }

class GroovesharkServiceSettings : public KCModule
{
    Q_OBJECT

public:
    explicit GroovesharkServiceSettings( QWidget *parent = 0, const QVariantList &args = QVariantList() );

    virtual ~GroovesharkServiceSettings();

    virtual void save();
    virtual void load();
    virtual void defaults();

private slots:
    void testLogin();
    void onAuthenticated();
    void onError( QNetworkReply::NetworkError code );
private:
    Ui::GroovesharkConfigWidget *m_configDialog;
    GroovesharkServiceConfig     m_config;

    QNetworkReply* m_authQuery;

private slots:
    void settingsChanged();
};

#endif // GROOVESHARKSERVICESETTINGS_H
