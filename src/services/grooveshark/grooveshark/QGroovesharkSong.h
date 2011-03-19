#ifndef QGROOVESHARK_SONG_H
#define QGROOVESHARK_SONG_H

#include "QGrooveshark.h"
#include <QObject>
#include <QString>

class QGroovesharkSong: public QObject
{
    Q_OBJECT

public:
    explicit QGroovesharkSong(QObject *parent = 0);

    QGrooveshark manager;

    QString songID;
    QString songName;
    QString name;
    QString artistName;
    QString artistID;
    QString albumName;
    QString albumID;
    bool isVerified;


    void fromJSON(QVariantMap songinfo);
    void markDownloaded();
    void mark30Seconds();
    void getStreamDetails();
    void getStreamURL();
    void download();

};

#endif
