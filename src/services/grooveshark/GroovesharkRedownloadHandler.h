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

#ifndef GROOVESHARK_REDOWNLOAD_HANDLER_H
#define GROOVESHARK_REDOWNLOAD_HANDLER_H

#include "GroovesharkAlbumDownloader.h"
#include "GroovesharkDownloadDialog.h"
#include "GroovesharkDownloadInfo.h"
#include "GroovesharkRedownloadDialog.h"

#include <QObject>

/**
This class handles the redownloading of previously downloaded albums

    @author Nikolaj Hald Nielsen <nhn@kde.org>
*/

class GroovesharkRedownloadHandler : public QObject {
Q_OBJECT
public:
    GroovesharkRedownloadHandler(QWidget * parent);

    ~GroovesharkRedownloadHandler();

    /**
     * Calls forth the redownload dialog.
     */
    void showRedownloadDialog();

signals:

    void reDownloadCompleted( bool success );

protected:

    QStringList GetPurchaseList( );

   /**
    * Attempt to get a list of previous purchases for an email.
    * If set, use the email from the grooveshark settings, otherwise, QueryMaker
    * the user (and then save it to settings)
    */
    void fetchServerSideRedownloadList();

    QWidget * m_parent;
    GroovesharkRedownloadDialog * m_redownloadDialog;
    GroovesharkDownloadDialog * m_downloadDialog;
    GroovesharkAlbumDownloader * m_albumDownloader;

    KIO::TransferJob * m_redownloadApiJob;

protected slots:

    void redownload( GroovesharkDownloadInfo info );
    void selectionDialogCancelled();
    void albumDownloadComplete( bool success );
    void redownloadApiResult( KJob* job );



};

#endif
