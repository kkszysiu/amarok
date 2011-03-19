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

#ifndef GROOVESHARKDOWNLOADDIALOG_H
#define GROOVESHARKDOWNLOADDIALOG_H

#include "ui_GroovesharkDownloadDialogBase.h"
#include "GroovesharkDownloadInfo.h"

#include <qmap.h>


/**
    Dialog for choosing download format and location. Also displays additional info from Grooveshark.com.
 
    @author Nikolaj Hald Nielsen <nhn@kde.org>
*/
class GroovesharkDownloadDialog : public QDialog, public Ui::groovesharkDownloadDialogBase
{
    Q_OBJECT

public:
    /**
     * Overridden constructor.
     * @param parent Pointer to the parent QWidget.
     * @param name Name of this widget.
     * @param modal Sets modal state.
     * @param fl Additional dialog flags.
     */
    explicit GroovesharkDownloadDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );

    /**
     * Destructor
     */
    ~GroovesharkDownloadDialog();
    /*$PUBLIC_FUNCTIONS$*/

    /**
     * Sets the current download info 
     * @param  the GroovesharkDownloadInfo class containing the information abut the 
     * download to display
     */
    void setDownloadInfo( GroovesharkDownloadInfo info );

signals:

    /**
     * Signal emitted when all needed info has been gathered and handler 
     * should start album download.
     * @param completedInfo A DownloadInfo object containing all needed information
     */
    void downloadAlbum( GroovesharkDownloadInfo completedInfo );

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    /*$PROTECTED_FUNCTIONS$*/
    GroovesharkDownloadInfo m_currentDownloadInfo;

protected slots:
    /*$PROTECTED_SLOTS$*/
    /**
     * Slot for recieving notification when the download button is clicked.
     */
    void downloadButtonClicked();

};

#endif

