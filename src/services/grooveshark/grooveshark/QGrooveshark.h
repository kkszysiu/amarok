#ifndef QGROOVESHARK_H
#define QGROOVESHARK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>

//namespace QGrooveshark {

class QGrooveshark: public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;

public:
    QGrooveshark();
    void doDownload(const QUrl &url);
    void authoriseUser(const QString &username, const QString &password);
    QString saveFileName(const QUrl &url);
    bool saveToDisk(const QString &filename, QIODevice *data);

    QString generateToken(const QString &method);

    void makeRequest(const QString &type, const QVariantMap &parameters);
    void startAuth();
    void getCommunicationToken();

    QString getDomain();
    QString getHomeURL();
    QString getTokenURL();
    QString getApiURL();
    QString getServiceURL();
    QString getRandomChars();
    QString getClientName();
    QString getClientVersion();
    QString getToken(const QString &method);

    QString session;

    QString token;
    //QDateTime lastTokenTime;
    QString uuid;
    QVariantMap userObject;

public slots:
    void downloadFinished(QNetworkReply *reply);

signals:
    void sessionReceived();
    void tokenReceived();
    void authenticationSucceed();
    void authenticationFailed(const QString &message);
};

//}

#endif
//#include "qgrooveshark.moc"
