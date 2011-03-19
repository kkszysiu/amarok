#include <QDebug>

#include "QGroovesharkSong.h"


QGroovesharkSong::QGroovesharkSong(QObject *parent) :
    QObject(parent)
{
}

void QGroovesharkSong::fromJSON(QVariantMap songinfo) {
//    qDebug() << "fromJSON:" << songinfo;
//    qDebug() << "AlbumID:" << songinfo["AlbumID"].toString();
//    qDebug() << "AlbumName:" << songinfo["AlbumName"].toString();
//    qDebug() << "ArtistID:" << songinfo["ArtistID"].toString();
//    qDebug() << "ArtistName:" << songinfo["ArtistName"].toString();
//    qDebug() << "IsVerified:" << songinfo["IsVerified"].toBool();
//    qDebug() << "Name:" << songinfo["Name"].toString();
//    qDebug() << "SongID:" << songinfo["SongID"].toString();
//    qDebug() << "SongName:" << songinfo["SongName"].toString();
    this->songID = songinfo["SongID"].toString();
    this->songName = songinfo["SongName"].toString();
    this->albumID = songinfo["AlbumID"].toString();
    this->albumName = songinfo["AlbumName"].toString();
    this->artistID = songinfo["ArtistID"].toString();
    this->albumName = songinfo["ArtistName"].toString();
    this->isVerified = songinfo["IsVerified"].toBool();
//    QString songID;
//    QString name;
//    QString artistName;
//    QString artistID;
//    QString albumName;
//    QString albumID;
//    bool isVerified;

}

void QGroovesharkSong::markDownloaded() {

}

void QGroovesharkSong::mark30Seconds() {

}

void QGroovesharkSong::getStreamDetails() {

}

void QGroovesharkSong::getStreamURL() {

}

void QGroovesharkSong::download() {

}
