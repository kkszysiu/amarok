/*
  Copyright (c) 2006  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>

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

#ifndef MAGNATUNETYPES_H
#define MAGNATUNETYPES_H


#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QList>


class MagnatuneArtist 
{

private:
    int m_id;
    QString m_name;
    QString m_genre;
    QString m_description;
    QString m_photoURL;
    QString m_homeURL;

public:
    MagnatuneArtist();

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
};


class MagnatuneAlbum 
{
private:
    int m_id;
    QString m_name;
    QString m_coverURL;
    QDate m_launchDate;
    QString m_albumCode;
    QString m_mp3Genre;
    QString m_magnatuneGenres;
    QString m_description;
    int m_artistId;

public:
    MagnatuneAlbum();

    void setId( int id );
    int getId() const;

    void setName( const QString &name );
    QString getName() const;

    void setCoverURL( const QString &coverURL );
    QString getCoverURL() const;

    void setLaunchDate( const QDate &launchDate );
    QDate getLaunchDate() const;

    void setAlbumCode( const QString &albumCode );
    QString getAlbumCode() const;

    void setMp3Genre( const QString &mp3Genre );
    QString getMp3Genre() const;

    void setMagnatuneGenres( const QString &magnatuneGenres );
    QString getMagnatuneGenres() const;

    void setArtistId( int artistId );
    int getArtistId() const;

    void setDescription( const QString description );
    QString getDescription();


};

class MagnatuneTrack 
{
private:
    int m_id;
    QString m_name;
    int m_trackNumber;
    int m_duration;
    QString m_hifiURL;
    QString m_lofiURL;
    int m_albumId;
    int m_artistId;
    QStringList m_moods;

public:
    MagnatuneTrack();

    void setId( int id );
    int getId() const;
 
    void setName( const QString &name );
    QString getName() const;

    void setTrackNumber( int trackNumber );
    int getTrackNumber() const;

    void setDuration( int duration );
    int getDuration() const;

    void setHifiURL( const QString &hifiURL );
    QString getHifiURL() const;

    void setLofiURL( const QString &lofiURL );
    QString getLofiURL() const;

    void setAlbumId( int albumId );
    int getAlbumId() const;

    void setArtistId( int artistId );
    int getArtistId() const;

    void setMoods( const QStringList &moods );
    QStringList getMoods() const;
};

typedef QList<MagnatuneArtist> MagnatuneArtistList;
typedef QList<MagnatuneAlbum> MagnatuneAlbumList;
typedef QList<MagnatuneTrack> MagnatuneTrackList;

#endif
