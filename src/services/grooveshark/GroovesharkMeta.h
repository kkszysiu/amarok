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

#ifndef GROOVESHARKMETA_H
#define GROOVESHARKMETA_H


#include "../ServiceMetaBase.h"
#include "../ServiceAlbumCoverDownloader.h"

#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>

class GroovesharkAlbumCoverDownloader;
class GroovesharkStore;

namespace Meta
{
    
class GroovesharkTrack  : public ServiceTrack
{
    Q_OBJECT
public:
    GroovesharkTrack( const QString &name );
    GroovesharkTrack( const QStringList &resultRow );

    QString lofiUrl();
    void setLofiUrl( const QString &url );

    QList<QString> moods();
    void setMoods(  QList<QString> moods );

    void setDownloadMembership();

    virtual QList< QAction *> customActions();
    virtual QList< QAction *> currentTrackActions();

    virtual QString sourceName();
    virtual QString sourceDescription();
    virtual QPixmap emblem();

    virtual bool isBookmarkable() const { return true; }
    virtual QString collectionName() const { return "Grooveshark.com"; }
    virtual bool simpleFiltering() const { return false; }

    void setOggUrl( const QString& url );
    QString oggUrl() const;

    void setAlbumPtr( Meta::AlbumPtr album );

public slots:
    void download();

private:
    QString m_lofiUrl;
    QString m_oggUrl;
    bool m_downloadMembership;
    QList<QString> m_moods;
    QAction * m_downloadAction;

};


class GroovesharkArtist : public ServiceArtist
{
private:

    QString m_photoUrl;
    QString m_groovesharkUrl;

public:
    GroovesharkArtist( const QString &name );
    GroovesharkArtist( const QStringList &resultRow );

    void setPhotoUrl( const QString &photoUrl );
    QString photoUrl() const;

    void setGroovesharkUrl( const QString &url );
    QString groovesharkUrl() const;

    virtual bool isBookmarkable() const { return true; }
    virtual QString collectionName() const { return "Grooveshark.com"; }
    virtual bool simpleFiltering() const { return false; }
};

class GroovesharkAlbum : public ServiceAlbumWithCover
{
    Q_OBJECT
private:
    QString m_coverUrl;
    int m_launchYear;
    QString m_albumCode;
    GroovesharkStore * m_store;
    bool m_downloadMembership;


public:
    GroovesharkAlbum( const QString &name );
    GroovesharkAlbum( const QStringList &resultRow );

    ~GroovesharkAlbum();
    
    virtual QString downloadPrefix() const { return "grooveshark"; }

    void setLaunchYear( int launchYear );
    int launchYear() const;

    virtual void setCoverUrl( const QString &coverUrl );
    virtual QString coverUrl() const;

    virtual KUrl imageLocation( int size = 1 ) { Q_UNUSED( size ); return KUrl( coverUrl() ); }
    
    void setAlbumCode(  const QString &albumCode );
    QString albumCode();

    virtual QList< QAction *> customActions();

    void setStore( GroovesharkStore * store );
    GroovesharkStore * store();

    void setDownloadMembership();

    virtual bool isBookmarkable() const { return true; }
    virtual QString collectionName() const { return "Grooveshark.com"; }
    virtual bool simpleFiltering() const { return false; }

public slots:
    void download();
    void addToFavorites();
private:
    QAction * m_downloadAction;
    QAction * m_addToFavoritesAction;
};

class GroovesharkGenre  : public ServiceGenre
{

public:
    GroovesharkGenre( const QString &name );
    GroovesharkGenre( const QStringList &resultRow );

    virtual bool isBookmarkable() const { return true; }
    virtual QString collectionName() const { return "Grooveshark.com"; }
    virtual bool simpleFiltering() const { return false; }
};

}

class GroovesharkMetaFactory : public ServiceMetaFactory
{

    public:
        enum { OGG = 0, MP3 = 1, LOFI = 2 };
        
        GroovesharkMetaFactory( const QString &dbPrefix, GroovesharkStore * store );
        virtual ~GroovesharkMetaFactory() {}

        virtual int getTrackSqlRowCount();
        virtual QString getTrackSqlRows();
        virtual Meta::TrackPtr createTrack( const QStringList &rows );

        virtual int getAlbumSqlRowCount();
        virtual QString getAlbumSqlRows();
        virtual Meta::AlbumPtr createAlbum( const QStringList &rows );

        virtual int getArtistSqlRowCount();
        virtual QString getArtistSqlRows();
        virtual Meta::ArtistPtr createArtist( const QStringList &rows );

    //virtual int getGenreSqlRowCount();
    //virtual QString getGenreSqlRows();
        virtual Meta::GenrePtr createGenre( const QStringList &rows );

        //stuff for supporting the new membership services at Grooveshark.com

        void setMembershipInfo ( const QString &prefix, const QString &userName, const QString &password );
        void setStreamType( int type );

    private:
        QString m_membershipPrefix;
        int m_streamType;

        QString m_userName;
        QString m_password;
        GroovesharkStore * m_store;
};


#endif
