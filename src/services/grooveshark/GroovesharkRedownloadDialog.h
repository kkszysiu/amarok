/****************************************************************************************
 * Copyright (c) 2006,2007 Nikolaj Hald Nielsen <nhn@kde.org>                           *
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

#ifndef GROOVESHARK_REDOWNLOAD_DIALOG_H
#define GROOVESHARK_REDOWNLOAD_DIALOG_H

#include "ui_GroovesharkRedownloadDialogBase.h"

#include "GroovesharkDownloadInfo.h"

#include <QStringList>

class GroovesharkRedownloadDialog : public QDialog, public Ui::groovesharkReDownloadDialogBase
{
    Q_OBJECT

public:
    explicit GroovesharkRedownloadDialog( QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0 );
    ~GroovesharkRedownloadDialog();
    /*$PUBLIC_FUNCTIONS$*/

    void setRedownloadItems( const QStringList &items );
    void setRedownloadItems( QList<GroovesharkDownloadInfo> previousPurchases );

signals:

    void redownload( const QString &downloadInfoFileName );
    void redownload( GroovesharkDownloadInfo info );
    void cancelled();

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    QMap <QTreeWidgetItem*, GroovesharkDownloadInfo> m_infoMap;

protected slots:
    /*$PROTECTED_SLOTS$*/
    void redownload();
    void selectionChanged();
    void reject();

};

#endif

