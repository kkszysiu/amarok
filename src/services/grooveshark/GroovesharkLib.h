#ifndef AUTH_H
#define AUTH_H

#include <QObject>
#include <QNetworkCookieJar>
#include <QHttp>
#include <QNetworkInterface>

/*
class MyObject : public QObject
{
    Q_OBJECT
public slots:
    void logMsg( QString aMessage )
    {
        qDebug() << "Log message:" << aMessage;
    }
    void toSend( QString aMessage )
    {
        qDebug() << "To sent:" << aMessage;
    }
    void done( GroovesharkLib &auth )
    {
        qDebug() << "I win at life!";
        qDebug() << auth.iUOKey;
    }
};
*/

class GroovesharkLib : public QObject
{
    Q_OBJECT
public:
    GroovesharkLib(QObject* parent = 0);
    ~GroovesharkLib();

    QString transformKey(QString aKey);
    void uoRequest(QString aNick, QString aNickGroovesharkLib, QString aPassword);

private:
    QString version(QString aData);
    void requestComplete(QString aNickGroovesharkLib, QString aData);

public:
    QString iUbi, iCid, iSid, iUid;
    QString iUOKey, iUONick;

signals:
    void send(QString aSend);
    void logMessage(QString aMessage);
    void success();
};

#endif


