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

#ifndef AMAROKGROOVESHARKSTORE_H
#define AMAROKGROOVESHARKSTORE_H


#include "core/support/Amarok.h"
#include "GroovesharkDownloadHandler.h"
#include "GroovesharkRedownloadHandler.h"
#include "GroovesharkXmlParser.h"
#include "GroovesharkDatabaseHandler.h"
#include "GroovesharkCollection.h"

#include "grooveshark/QGrooveshark.h"

#include "../ServiceBase.h"

#include <kio/job.h>
#include <kio/jobclasses.h>

#include <QCheckBox>
#include <QComboBox>
#include <khbox.h>
#include <QPushButton>
#include <kvbox.h>


class GroovesharkInfoParser;

class GroovesharkServiceFactory : public ServiceFactory
{
    Q_OBJECT

    public:
        GroovesharkServiceFactory( QObject *parent, const QVariantList &args );
        virtual ~GroovesharkServiceFactory() {}

        virtual void init();
        virtual QString name();
        virtual KConfigGroup config();

        virtual bool possiblyContainsTrack( const KUrl &url ) const { return url.url().contains( "grooveshark.com", Qt::CaseInsensitive ); }
};


/**
Amarok browser that displays all the music available at grooveshark.com and makes it available for previewing and purchasing.
Implemented as a singleton

@author Nikolaj Hald Nielsen <nhn@kde.org>
*/
class GroovesharkStore: public ServiceBase
{
    Q_OBJECT

public:
     /**
     * Constructor
     */
    GroovesharkStore( GroovesharkServiceFactory* parent, const char *name );
    /**
     * Destructor
     */
    ~GroovesharkStore();

    void setMembership( int type, const QString &username,  const QString &password );

    /**
     * OGG, MP3 or LOFI
     */
    void setStreamType( int );
    
     /**
     * Do not do expensive initializations before we are actually shown
     */
    void polish();

    QGrooveshark manager;

    virtual Collections::Collection * collection() { return m_collection; }

    virtual QString messages();
    virtual QString sendMessage( const QString &message );

public slots:
    /**
    * Slot for catching cancelled list downloads
    */
    void onSearchEntered( const QString &message );
    void gotUsernameAndPassword(const QString &username, const QString &password, bool keep);

    void loggedIn();
    void userAuthenticated();
    void onSearchResultsReceived(QList<QGroovesharkSong*> songlist);

    void onSearchEverything() { m_searchType = 0; }
    void onSearchArtists() { m_searchType = 1; }
    void onSearchAlbums() { m_searchType = 2; }
    void onSearchPlaylists() { m_searchType = 3; }
    void onSearchUsers() { m_searchType = 4; }

    void listDownloadCancelled();

    void download( Meta::GroovesharkTrack * track );

    void download( Meta::GroovesharkAlbum * album );

    void showFavoritesPage();
    void showHomePage();
    void showRecommendationsPage();

    void addToFavorites( const QString &sku );
    void removeFromFavorites( const QString &sku );

    void showSignupDialog();

private slots:
    /**
     * Slot called when the download album button is clicked. Starts a download
     */
    void download();

    void download( const QString &sku );
    
    void downloadCurrentTrackAlbum();

    /**
     * Slot for recieving notification that the update button has been clicked.
     */
    void updateButtonClicked();


    /**
     * Slot for recieving notification when the Grooveshark xml file has been downloaded.
     * Triggers a parse of the file to get the info added to the databse
     * @param downLoadJob The calling download Job
     */
    void listDownloadComplete( KJob* downLoadJob );


    /**
     * Slot called when the parsing of the Magnatuin xml file is completed.
     * Triggers an update of the list view and the genre combo box
     */
    void doneParsing();

    /**
     * Starts the process of redownloading a previously bought album
     */
    void processRedownload();

    /**
     * Slot for recieving notifications of completed download operations
     * @param success Was the operation a success?
     */
    void downloadCompleted( bool success );


    /**
     * Adds all tracks with a common mood to the playlist
     * @param mood The mood of the tracks to add
     */
    void addMoodyTracksToPlaylist( const QString &mood, int count );


     /**
     * Checks if download button should be enabled
     * @param selection the new selection
     * @param deseleted items that were previously selected but have been deselected
     */
    void itemSelected( CollectionTreeItem * selectedItem );


    void moodMapReady( QMap<QString, int> map );
    void moodyTracksReady( Meta::TrackList tracks );

    void timestampDownloadComplete( KJob * job );
    void favoritesResult( KJob* addToFavoritesJob );

private:
    /**
     * Helper function that initializes the button panel below the list view
     */
    void initBottomPanel();

    /**
     * Helper function that initializes the genre selection panel above the list view
     */
    void initTopPanel();

    /**
     * Starts downloading an updated track list xml file from
     * http://grooveshark.com/info/album_info.xml
     * @return Currently always returns true
     */
    bool updateGroovesharkList();

    void checkForUpdates();

    /**
     * Adds a grooveshark preview track to the playlist.
     * @param item The track to add
     */
    //void addTrackToPlaylist ( Meta::GroovesharkTrack  *item );

    static GroovesharkStore *s_instance;

    QString m_currentInfoUrl;
    QMenu *m_popupMenu;
    int m_searchType;
    GroovesharkDownloadHandler *m_downloadHandler;
    GroovesharkRedownloadHandler *m_redownloadHandler;

    QPushButton *m_signIn;

    QAction * m_updateAction;

    QComboBox   *m_genreComboBox;
    bool         m_downloadInProgress;

    Meta::GroovesharkAlbum * m_currentAlbum;

    KIO::FileCopyJob * m_listDownloadJob;
    KIO::StoredTransferJob* m_updateTimestampDownloadJob;
    KIO::StoredTransferJob* m_favoritesJob;

    Collections::GroovesharkCollection * m_collection;

    QString m_tempFileName;

    bool m_isMember;
    int m_membershipType;
    QString m_username;
    QString m_password;

    int m_streamType;

    QSortFilterProxyModel *m_proxyModel;

    QItemSelectionModel *m_selectionModel;

    qulonglong m_groovesharkTimestamp;
    //ServiceSqlRegistry * m_registry;

    GroovesharkInfoParser * m_groovesharkInfoParser;

    QDialog *m_signupInfoWidget;
};


#endif
