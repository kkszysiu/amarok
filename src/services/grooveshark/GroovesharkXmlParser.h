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

#ifndef GROOVESHARKXMLPARSER_H
#define GROOVESHARKXMLPARSER_H

#include "GroovesharkDatabaseHandler.h"
#include "GroovesharkMeta.h"

#include <qdom.h>
#include <QString>
#include <QDomElement>
#include <QMap>

#include <threadweaver/Job.h>

/**
* Parser for the XML file from http://grooveshark.com/info/album_info.xml
*
* @author Nikolaj Hald Nielsen
*/
class GroovesharkXmlParser : public ThreadWeaver::Job
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param fileName The file to parse
     * @return Pointer to new object
     */
    GroovesharkXmlParser( const QString &fileName );

    /**
     * The function that starts the actual work. Inherited from ThreadWeaver::Job
     * Note the work is performed in a separate thread
     */
    void run();

    /**
     * Destructor
     * @return none
     */
    ~GroovesharkXmlParser();

    /**
     * Reads, and starts parsing, file. Should not be used directly.
     * @param filename The file to read
     */
    void readConfigFile( const QString &filename );


    void setDbHandler( GroovesharkDatabaseHandler * dbHandler );

signals:

    /**
     * Signal emmited when parsing is complete.
     */
    void doneParsing();

    private slots:
        
    /**
     * Called when the job has completed. Is executed in the GUI thread
     */
    void completeJob();

private:

    QMap<QString, int> artistNameIdMap;

    QString m_currentArtist;
    QString m_currentArtistGenre;

    /**
     * Parses a DOM element
     * @param e The element to parse
     */
    void parseElement( const QDomElement &e );

    /**
     * Parses all children of a DOM element
     * @param e The element whose children is to be parsed
     */
    void parseChildren( const QDomElement &e );

    /**
     * Parse a DOM element representing an album
     * @param e The album element to parse
     */
    void parseAlbum( const QDomElement &e );

    /**
     * Parse a DOM element representing a track
     * @param e The track element to parse
     */
    void parseTrack( const QDomElement &e );

    /**
     * Parse the moods of a track
     * @param e The moods element to parse
     */
    void parseMoods( const QDomElement &e );

    Meta::GroovesharkAlbum *m_pCurrentAlbum;
    Meta::GroovesharkArtist *m_pCurrentArtist;
    QList<Meta::GroovesharkTrack *> m_currentAlbumTracksList;
    QStringList m_currentTrackMoodList;

    QString m_sFileName;

    int m_nNumberOfTracks;
    int m_nNumberOfAlbums;
    int m_nNumberOfArtists;

    GroovesharkDatabaseHandler * m_dbHandler;


};

#endif
