/***************************************************************************************
* Copyright (c) 2009 Nathan Sala <sala.nathan@gmail.com>                               *
* Copyright (c) 2009 Oleksandr Khayrullin <saniokh@gmail.com>                          *
* Copyright (c) 2009-2010 Joffrey Clavel <jclavel@clabert.info>                        *
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

#ifndef SIMILARARTISTSENGINE_H
#define SIMILARARTISTSENGINE_H

#include "NetworkAccessManagerProxy.h"
#include "context/ContextObserver.h"
#include "context/DataEngine.h"
#include "context/applets/similarartists/SimilarArtist.h"
#include "core/engine/EngineObserver.h"
#include "core/meta/Meta.h"

#include <QLocale>

using namespace Context;

/**
 *  This class provide SimilarArtists data for use in the SimilarArtists context applet.
 *  It gets its information from the API lastfm.
 */
class SimilarArtistsEngine : public DataEngine, public Meta::Observer, public Engine::EngineObserver
{
    Q_OBJECT

public:

    /**
     * Construct the engine
     * @param parent The object parent to this engine     
     */
    SimilarArtistsEngine( QObject *parent, const QList<QVariant> &args );

    /**
     * Destroy the dataEngine
     */
    virtual ~SimilarArtistsEngine();

    /**
    * Fetches the similar artists for an artist thanks to the LastFm WebService
    * @param artistName the name of the artist
    * @return a map with the names of the artists with their match rate
    */
    QMap<int, QString> similarArtists( const QString &artistName );

    /**
     * Fetches the similar artists for an artist thanks to the LastFM WebService
     * Store this in the similar artist list of this class
     * @param artistName the name of the artist
     */
    void similarArtistsRequest( const QString &artistName );

    // reimplemented from Meta::Observer
    using Observer::metadataChanged;
    void metadataChanged( Meta::TrackPtr track );

    // reimplemented from EngineObserver
    void engineNewTrackPlaying();

protected:
    bool sourceRequestEvent( const QString &name );

private:
    QString descriptionLocale() const;

    QLocale m_descriptionLang;
    QString m_descriptionWideLang;

    /**
     * Prepare the calling of the similarArtistsRequest method.
     * Launch when the track played on amarok has changed.
     */
    void update();

    /**
     * Fetches the description of the artist artistName on the LastFM API.
     * @param artistName the name of the artist
     */
    void artistDescriptionRequest( const QString &artistName );

    /**
     * Fetches the the most known artist track of the artist artistName on the LastFM API
     * @param artistName the name of the artist
     */
    void artistTopTrackRequest( const QString &artistName );

    /**
     * The max number of similar artists to get
     */
    int m_maxArtists;

    /**
     * The number of artists description fetched
     */
    int m_descriptionArtists;

    /**
     * The number of top tracks fetched
     */
    int m_topTrackArtists;

    /**
     * The job for download the data from the lastFm API
     */
    KUrl m_similarArtistsUrl;

    /**
     * The list of jobs that fetch the artists description on the lastFM API
     */
    QSet<KUrl> m_artistDescriptionUrls;

    /**
     * The list of jobs that fetch the most known artists tracks on the lastFM API
     */
    QSet<KUrl> m_artistTopTrackUrls;

    /**
     * The current track played on amarok
     */
    Meta::TrackPtr m_currentTrack;

    /**
     * The list of similar artists fetched on the last fm API
     */
    SimilarArtist::List m_similarArtists;

    /**
     * The artist, whose research is similar artists.
     */
    QString m_artist;

private slots:

    /**
     * Parse the xml fetched on the lastFM API.
     * Launched when the download of the data are finished.
     */
    void parseSimilarArtists( const KUrl &url, QByteArray data, NetworkAccessManagerProxy::Error e );

    /**
     * Parse the xml fetched on the lastFM API for the similarArtist description.
     * Launched when the download of the data are finished and for each similarArtists.
     */
    void parseArtistDescription( const KUrl &url, QByteArray data, NetworkAccessManagerProxy::Error e );

    /**
     * Parse the xml fetched on the lastFM API for the similarArtist most known track.
     * Launched when the download of the data are finished and for each similarArtists.
     */
    void parseArtistTopTrack( const KUrl &url, QByteArray data, NetworkAccessManagerProxy::Error e );

};

#endif // SIMILARARTISTSENGINE_H


