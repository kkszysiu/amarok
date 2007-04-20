/*
  Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef JAMENDOTYPES_H
#define JAMENDOTYPES_H


#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QList>


class JamendoArtist 
{

private:
    int m_id;
    QString m_name;
    QString m_country;
    QString m_description;
    QString m_photoURL;
    QString m_jamendoURL;
    QString m_homeURL;

public:
    JamendoArtist();

    void setId( int id );
    int getId() const;

    void setName( const QString &name );
    QString getName() const;

    void setDescription( const QString &description );
    QString getDescription() const;

    void setPhotoURL( const QString &photoURL );
    QString getPhotoURL() const;
  
    void setHomeURL( const QString &homeURL );
    QString getHomeURL() const;

    void setJamendoURL( const QString &jamendoURL );
    QString getJamendoURL() const;
};


class JamendoAlbum 
{
private:
    int m_id;
    float m_popularity;
    QString m_name;
    QString m_coverURL;
    QDate m_launchDate;
    QString m_genre;
    QString m_jamendoTags;
    QString m_description;
    int m_artistId;
    QStringList m_tags;

public:
    JamendoAlbum();

    void setId( int id );
    int getId() const;

    void setPopularity( float popularity );
    float getPopularity() const;

    void setName( const QString &name );
    QString getName() const;

    void setCoverURL( const QString &coverURL );
    QString getCoverURL() const;

    void setLaunchDate( const QDate &launchDate );
    QDate getLaunchDate() const;

    void setJamendoTags( const QStringList &tags  );
    QStringList getJamendoTags() const;

    void setGenre( const QString &genre );
    QString getGenre() const;

    void setArtistId( int artistId );
    int getArtistId() const;

    void setDescription( const QString description );
    QString getDescription();


};

class JamendoTrack 
{
private:
    int m_id;
    QString m_name;
    int m_trackNumber;
    int m_duration;
    QString m_lofiURL;
    int m_albumId;
    int m_artistId;


public:
    JamendoTrack();

    void setId( int id );
    int getId() const;
 
    void setName( const QString &name );
    QString getName() const;

    void setTrackNumber( int trackNumber );
    int getTrackNumber() const;

    void setDuration( int duration );
    int getDuration() const;

    void setLofiURL( const QString &lofiURL );
    QString getLofiURL() const;

    void setAlbumId( int albumId );
    int getAlbumId() const;

    void setArtistId( int artistId );
    int getArtistId() const;

};

typedef QList<JamendoArtist> JamendoArtistList;
typedef QList<JamendoAlbum> JamendoAlbumList;
typedef QList<JamendoTrack> JamendoTrackList;

#endif
