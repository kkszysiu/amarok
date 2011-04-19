
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QDateTime>
#include <QUrl>
#include <QDebug>
#include <QUuid>
#include <QVariant>
#include <QCryptographicHash>
#include "qjson/parser.h"
#include "qjson/serializer.h"

#include "QGrooveshark.h"
#include "QGroovesharkSong.h"

//namespace QGrooveshark;

QGrooveshark::QGrooveshark()
{
    this->uuid = QUuid::createUuid().toString();
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));
}

QString QGrooveshark::getDomain() {
    return QString("grooveshark.com");
}

QString QGrooveshark::getHomeURL() {
    return QString("http://listen.").append(getDomain());
}

QString QGrooveshark::getTokenURL() {
    return QString("http://cowbell.").append(getDomain()).append("/more.php");
}

QString QGrooveshark::getApiURL() {
    return QString("http://cowbell.").append(getDomain()).append("/more.php");
}

QString QGrooveshark::getServiceURL() {
    return QString("http://cowbell.").append(getDomain()).append("/service.php");
}

QString QGrooveshark::getRandomChars() {
    //return "1234567890abcdef";
    return "123456";
}

QString QGrooveshark::getClientName() {
    return "htmlshark";
}

QString QGrooveshark::getClientVersion() {
    //return "20101012.37";
    return "20101222.41";
}

QString QGrooveshark::generateToken(const QString &method) {
    //if( this->lastTokenTime.isNull() || ( ( QDateTime::currentDateTime().toTime_t() - this->lastTokenTime.addMSecs( 1000 ).toTime_t() ) <= 1000 ) ) {
        QString hashString = QString(method)
                                .append(":")
                                .append(this->token)
                                .append(":quitStealinMahShit:")
                                .append(getRandomChars());
        qDebug() << "hashString:" << hashString;
        
        QByteArray hash = QCryptographicHash::hash(hashString.toAscii(), QCryptographicHash::Sha1);
        qDebug() << "Generated hash string:" << getRandomChars().append(hash.toHex());
        return getRandomChars().append(hash.toHex());
    //} else {
    //    return QString();
    //}
}


 void QGrooveshark::doDownload(const QUrl &url)
 {
     QNetworkRequest request(url);
     QNetworkReply *reply = manager.get(request);
     
     currentDownloads.append(reply);
 }

void QGrooveshark::makeRequest(const QString &type, const QVariantMap &parameters = QVariantMap())
{
    /*
     * QVariant -> JSON
     */
    QVariantMap jCountry;
    jCountry.insert("ID", "174");
    jCountry.insert("CC3", "35184372088832");
    jCountry.insert("CC2", "0");
    jCountry.insert("CC4", "0");
    jCountry.insert("CC1", "0");
    jCountry.insert("IPR", "11282");
    
    QVariantMap jHeader;
    if (type != "getCommunicationToken") {
        jHeader.insert("token", generateToken(type));
    }
    jHeader.insert("clientRevision", getClientVersion());
    jHeader.insert("uuid", this->uuid);
    jHeader.insert("session", this->session);
    jHeader.insert("country", jCountry);
    jHeader.insert("privacy", 1);
    jHeader.insert("client", getClientName());
    
    QVariantMap jData;
    jData.insert("parameters", parameters);
    jData.insert("method", type);
    jData.insert("header", jHeader);
    
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(jData);
    
    qDebug() << "JSON:" << json;
    
    QUrl url;
    if (type == "getCommunicationToken") {
        url = getTokenURL().append("?").append(type);
    } else if (type == "authenticateUser") {
        qDebug() << QString("https://listen.").append(getDomain()).append("/more.php?").append(type);
        url = QString("https://listen.").append(getDomain()).append("/more.php?").append(type);
    } else {
        url = getHomeURL().append("/more.php?").append(type);
    }

    qDebug() << "Request to:" << url;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, json.size());
    request.setRawHeader(QString("User-Agent").toAscii(), QString("Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12").toAscii());
    request.setRawHeader(QString("Content-Type").toAscii(), QString("application/json").toAscii());
    request.setRawHeader(QString("Referer").toAscii(), QString("http://listen.grooveshark.com/main.swf?cowbell=fe87233106a6cef919a1294fb2c3c05f").toAscii());
    QNetworkReply *reply = manager.post(request, json);
    
    currentDownloads.append(reply);
}

void QGrooveshark::getCommunicationToken() {
    QByteArray hash = QCryptographicHash::hash(this->session.toAscii(), QCryptographicHash::Md5);
    qDebug() << "secretKey session:" << hash.toHex();
    
    QVariantMap jParameters;
    jParameters.insert("secretKey", hash.toHex());

    makeRequest("getCommunicationToken", jParameters);
}

void QGrooveshark::authoriseUser(const QString &username, const QString &password)
{
    qDebug() << "authoriseUser";
    QVariantMap jParameters;
    jParameters.insert("password", password);
    jParameters.insert("savePassword", 0);
    jParameters.insert("username", username);

    makeRequest("authenticateUser", jParameters);
}

void QGrooveshark::startAuth() {
    QNetworkReply *reply = manager.get(QNetworkRequest((QUrl(getHomeURL()))));

    currentDownloads.append(reply);
}

void QGrooveshark::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    qDebug() << "URL:" << url;
    QJson::Parser parser;
    bool ok;
    
    if (reply->error()) {
        qDebug() << "Download of" << url << "failed: " << reply->errorString();
        return;
    }
    QString data = reply->readAll();

    if ( url.hasEncodedQueryItem("getCommunicationToken") ) {               // getCommunicationToken
        QVariantMap result = parser.parse (data.toAscii(), &ok).toMap();
        if (!ok) {
            qFatal("An error occurred during parsing");
        }
        this->token = result["result"].toString();
        emit tokenReceived();

    } else if ( url.hasEncodedQueryItem("authenticateUser") ) {             // authenticateUser
        QVariantMap result = parser.parse (data.toAscii(), &ok).toMap();
        if (!ok) {
            qFatal("An error occurred during parsing");
        }
        qDebug() << data;
        if(result["result"].isValid()) {
            userObject = result;
            emit authenticationSucceed();
        } else {
            if (result["fault"].isValid()) {
                QVariantMap fault = result["fault"].toMap();
                emit authenticationFailed(fault["message"].toString());
            } else {
                emit authenticationFailed("Unknown error");
            }
        }

    } else if ( url.hasEncodedQueryItem("getSearchResultsEx") ) {             // getSearchResultsEx
        //qDebug() << "getSearchResultsEx headers:" << reply->request().rawHeaderList();
        //qDebug() << "getSearchResultsEx:" << data;
        QList<QGroovesharkSong*> songlist;

        QVariantMap result = parser.parse (data.toAscii(), &ok).toMap();
        if (!ok) {
            qFatal("An error occurred during parsing");
        }

        if (result["result"].isValid()) {
            QVariantMap jresult = result["result"].toMap();
            QVariantList jresultlist = jresult["result"].toList();
            //qDebug() << jresultlist;
            foreach(QVariant jsong, jresultlist) {
                //qDebug() << "song:\n\n" << jsong;
                QVariantMap songinfo = jsong.toMap();
                QGroovesharkSong *song = new QGroovesharkSong();
                //QGroovesharkSong song;
                song->fromJSON(songinfo);
                songlist.append(song);
            }
            emit onSearchResultsReceived(songlist);

        } else {
            qWarning() << "Data is not valid";
        }

    } else if (  url == QUrl(getHomeURL()) ) {                               // homepage
        QRegExp rx("PHPSESSID=([A-Za-z_0-9]*)");
        int pos = rx.indexIn(reply->rawHeader("Set-Cookie"));
        if (pos > -1) {
            QString sessid = rx.cap(1);
            qDebug() << "sessid:" << sessid;
            this->session = sessid;

            // there we could get comm token because we need it anyway for correct logging procedure :)
            getCommunicationToken();
            emit sessionReceived();
        }

    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();

    //if (currentDownloads.isEmpty())
    // all downloads finished
    //QCoreApplication::instance()->quit();
}
