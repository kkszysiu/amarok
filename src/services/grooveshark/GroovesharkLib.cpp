#include "auth.h"
#include <QEventLoop>
#include <QNetworkReply>
#include <QHostInfo>
#include <QUrl>
#include <QDomDocument>
#include <QMessageBox>
#include <QTextCodec>

GroovesharkLib::GroovesharkLib(QObject *parent) : QObject(parent)
{
    //
}

GroovesharkLib::~GroovesharkLib()
{
    //
}

static bool testHosts()
{
    QHostInfo info = QHostInfo::fromName("czat.onet.pl");
    if(info.error() != QHostInfo::NoError)
        return false;
    info = QHostInfo::fromName("secure.onet.pl");
    if(info.error() != QHostInfo::NoError)
        return false;
    info = QHostInfo::fromName("kropka.onet.pl");
    if(info.error() != QHostInfo::NoError)
        return false;
    else return true;
}

static QString SyncHttpRequest(QNetworkAccessManager* accessManager, QString uri, QString content)
{
    QEventLoop eventLoop;
    QNetworkReply* reply;

    if(content.isEmpty())
        reply = accessManager->get(QNetworkRequest(QUrl(uri)));
    else
        reply = accessManager->post(QNetworkRequest(QUrl(uri)), content.toAscii());

    reply->ignoreSslErrors();
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    QByteArray dArr = reply->readAll();
    QString data = QTextCodec::codecForName("ISO-8859-2")->makeDecoder()->toUnicode(dArr);
    QString redir = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    reply->deleteLater();

    if(!redir.isEmpty())
        SyncHttpRequest(accessManager, redir, QString::null);

    return data;
}

static QString GetKropka(const QString& data)
{
    if(data.isEmpty())
        return QString::null;
    static const QString find1 = "onetKropka.src = '";
    static const QString find2 = "'";
    int start = data.indexOf(find1)+find1.length();
    int end = data.indexOf(find2, start);
    return data.mid(start, end-start);
}

void GroovesharkLib::uoRequest(QString aNick, QString aNickGroovesharkLib, QString aPassword)
{
    QString version;
    QNetworkAccessManager accessManager;
    QNetworkCookieJar* cookieJar = new QNetworkCookieJar();
    accessManager.setCookieJar(cookieJar);

    if(aNick.startsWith('~'))
        aNick = aNick.mid(1);

    QString random;
    while(random.length() < 11)
    {
        QChar c = QChar((qrand() % 9) + 48);
        random.append(c.toAscii());
    }

    bool override = false;
    //if(QCMSettings::Current()->Get("Override") == QString("TRUE"))
    //    override = true;

    if(!testHosts())
    {
        emit logMessage(tr("Onet servers not avaliable"));
        return;
    }

    // chat
    QString kropka = SyncHttpRequest(&accessManager, "http://czat.onet.pl/chat.html",
                                     "ch=&n=&p=&category=0");
    kropka = GetKropka(kropka);

    // deploy
    version = SyncHttpRequest(&accessManager, "http://czat.onet.pl/_s/deployOnetCzat.js", QString::null);
    version = this->version(version);
    version = QString("1.1(%1 - R)").arg(version);

    QString nickLength = QString("%1").arg(aNick.length());
    QString versionLength = QString("%1").arg(version.length());

    // kropka
    kropka += "&RI=&C1=&CL=std153&CS=1024x768&CW=1024x768&DU=http://czat.onet.pl/chat.html&DR=http://czat.onet.pl";
    SyncHttpRequest(&accessManager, kropka, QString::null);

    // behavioral targeting - RC
    SyncHttpRequest(&accessManager, QString("http://rc.onetwl.pl/Get/onet/JS/GetRcmd.js?ord=%1").arg(random),
                    QString::null);

    // behavioral targeting - TR
    QString contentGroup = "";
    QString params = "&CustomerId=onet&WebsiteId=czat.onet.pl&AC=on";
    SyncHttpRequest(&accessManager,
                    QString("http://tr.onetwl.pl/Cnt/onet/CP/%1?ord=%2%3").arg(contentGroup).arg(random)
                    .arg(params), QString::null);

    // full
    SyncHttpRequest(&accessManager, "http://kropka.onet.pl/_s/kropka/1?DV=czat/applet/FULL", QString::null);
    // sk
    SyncHttpRequest(&accessManager, "http://czat.onet.pl/sk.gif", QString::null);

    // for registered nick
    if(!aPassword.isEmpty())
    {
        /*QString secKropka;

        // secure
        secKropka = SyncHttpRequest(&accessManager, "http://secure.onet.pl/", QString::null);
        secKropka = GetKropka(secKropka);

        // secure kropka
        secKropka += "&RI=&C1=&CL=std153&SX=secure.onet.pl&CS=&CW=&DU=http://secure.onet.pl/&DR=";
        SyncHttpRequest(&accessManager, secKropka, QString::null);*/

        // secure login
        QString data = QString("r=&url=&login=%1&haslo=%2&app_id=20&ssl=1&ok=1").arg(aNick).arg(aPassword);
        QString result = SyncHttpRequest(&accessManager, "https://secure.onet.pl/mlogin.html", data);

        /*if(override)
        {
            data = QString("api_function=userOverride&params=a:1:{s:4:\"nick\";s:%1:\"%2\";}").
                   arg(nickLength).arg(aNick);
            SyncHttpRequest(&accessManager, "http://czat.onet.pl/include/ajaxapi.xml.php3", data);
        }*/

        // get uo
        data = QString("api_function=getUoKey&params=a:3:{s:4:\"nick\";s:%1:\"%2\";s:8:\"tempNick\";i:0;s:7:\"version\";s:%3:\"%4\";}").
               arg(nickLength).arg(aNick).arg(versionLength).
                arg(version);
        QString getUO = SyncHttpRequest(&accessManager, "http://czat.onet.pl/include/ajaxapi.xml.php3",
                                        data);

        requestComplete(aNickGroovesharkLib, getUO);
    }
    else    // not registered - tylda
    {
        QString data;
        data = QString("api_function=getUoKey&params=a:3:{s:4:\"nick\";s:%1:\"%2\";s:8:\"tempNick\";i:1;s:7:\"version\";s:%3:\"%4\";}").arg(nickLength).arg(aNick).
                arg(versionLength).arg(version);
        QString getUO = SyncHttpRequest(&accessManager, "http://czat.onet.pl/include/ajaxapi.xml.php3", data);
        requestComplete(aNickGroovesharkLib, getUO);
    }

    QList<QNetworkCookie> cookies = accessManager.cookieJar()->cookiesForUrl(QUrl("http://czat.onet.pl"));
    for(QList<QNetworkCookie>::iterator i = cookies.begin(); i != cookies.end(); ++i)
    {
        QString key = i->name();
        if(key == "onet_ubi")
            this->iUbi = i->value();
        else if(key == "onet_cid")
            this->iCid = i->value();
        else if(key == "onet_sid")
            this->iSid = i->value();
        else if(key == "onet_uid")
            this->iUid = i->value();
    }

    delete cookieJar;

}

QString GroovesharkLib::transformKey(QString s)
{
    static int f1[78] = {
        29, 43, 7, 5, 52, 58, 30, 59, 26, 35,
        35, 49, 45, 4, 22, 4, 0, 7, 4, 30,
        51, 39, 16, 6, 32, 13, 40, 44, 14, 58,
        27, 41, 52, 33, 9, 30, 30, 52, 16, 45,
        43, 18, 27, 52, 40, 52, 10, 8, 10, 14,
        10, 38, 27, 54, 48, 58, 17, 34, 6, 29,
        53, 39, 31, 35, 60, 44, 26, 34, 33, 31,
        10, 36, 51, 44, 39, 53, 5, 56
    };

    static int f2[] = {
        7, 32, 25, 39, 22, 26, 32, 27, 17, 50,
        22, 19, 36, 22, 40, 11, 41, 10, 10, 2,
        10, 8, 44, 40, 51, 7, 8, 39, 34, 52,
        52, 4, 56, 61, 59, 26, 22, 15, 17, 9,
        47, 38, 45, 10, 0, 12, 9, 20, 51, 59,
        32, 58, 19, 28, 11, 40, 8, 28, 6, 0,
        13, 47, 34, 60, 4, 56, 21, 60, 59, 16,
        38, 52, 61, 44, 8, 35, 4, 11
    };

    static int f3[] = {
        60, 30, 12, 34, 33, 7, 15, 29, 16, 20,
        46, 25, 8, 31, 4, 48, 6, 44, 57, 16,
        12, 58, 48, 59, 21, 32, 2, 18, 51, 8,
        50, 29, 58, 6, 24, 34, 11, 23, 57, 43,
        59, 50, 10, 56, 27, 32, 12, 59, 16, 4,
        40, 39, 26, 10, 49, 56, 51, 60, 21, 37,
        12, 56, 39, 15, 53, 11, 33, 43, 52, 37,
        30, 25, 19, 55, 7, 34, 48, 36
    };

    static int p1[] = {
        11, 9, 12, 0, 1, 4, 10, 13, 3, 6,
        7, 8, 15, 5, 2, 14
    };

    static int p2[] = {
        1, 13, 5, 8, 7, 10, 0, 15, 12, 3,
        14, 11, 2, 9, 6, 4
    };

    int ai[16];
    int ai1[16];

    if (s.length() < 16)
        return QString::null;

    for (int i = 0; i < 16; i++)
    {
        int c = s[i].toAscii();
        ai[i] = (c > 57 ? c > 90 ? (c - 97) + 36 : (c - 65) + 10 : c - 48);
    }

    for (int i = 0; i < 16; i++)
        ai[i] = f1[ai[i] + i];

//  arraycopy
    for (int i = 0; i < 16; i++) ai1[i] = ai[i];

    for (int i = 0; i < 16; i++)
        ai[i] = (ai[i] + ai1[p1[i]]) % 62;

    for (int i = 0; i < 16; i++)
        ai[i] = f2[ai[i] + i];

//  arraycopy
    for (int i = 0; i < 16; i++) ai1[i] = ai[i];

    for (int i = 0; i < 16; i++)
        ai[i] = (ai[i] + ai1[p2[i]]) % 62;

    for (int i = 0; i < 16; i++)
        ai[i] = f3[ai[i] + i];

    for (int i = 0; i < 16; i++)
    {
        int j = ai[i];
        ai[i] = j >= 10 ? j >= 36 ? (97 + j) - 36 : (65 + j) - 10 : 48 + j;
    }

//  result
    QString strResult;
    for (int i = 0; i < 16; i++)
        strResult += ai[i];

    return strResult;
}

QString GroovesharkLib::version(QString aData)
{
    if(!aData.isEmpty())
    {
        if(aData.indexOf("OnetCzatLoader") != -1)
        {
            static const QString find1 = "signed-OnetCzatLoader-";
            static const QString find2 = ".jar";
            int start = aData.indexOf(find1)+find1.length();
            int end = aData.indexOf(find2, start);
            QString version = aData.mid(start, end-start);
            if(!version.isEmpty() && version.length() > 0 && version.length() < 20)
                return version;
        }
    }

    return "20101008-1609";
}

void GroovesharkLib::requestComplete(QString aNickGroovesharkLib, QString aData)
{
    if(aData.isEmpty())
    {
        emit logMessage("Error: GroovesharkLiborization failed!");
        return;
    }
    QDomDocument doc;
    doc.setContent(aData);

    /* <?xml version="1.0" encoding="ISO-8859-2"?>
    <root>
        <uoKey>LY9j2sXwio0G_yo3PdpukDL8iZJGHXKs</uoKey>
        <zuoUsername>~Succubi_test</zuoUsername>
        <error err_code="TRUE"  err_text="wartość prawdziwa" >
        </error>
    </root> */
    QDomNodeList list = doc.documentElement().elementsByTagName("uoKey");
    if(!list.isEmpty())
    {
        QDomNodeList err = doc.documentElement().elementsByTagName("error");
        if(!err.isEmpty() && err.at(0).attributes().contains("err_code") &&
           err.at(0).attributes().namedItem("err_code").toAttr().value() == "TRUE")
        {
            iUOKey = list.at(0).toElement().text();
            iUONick = doc.elementsByTagName("zuoUsername").item(0).toElement().text();
            emit send(QString("NICK %1").arg(aNickGroovesharkLib));
            emit send("AUTHKEY");
            emit logMessage("Info: completing request succeeded");
            emit success();
            return;
        }
        else
        {
            emit logMessage("Error: GroovesharkLiborization failed.");
        }
    }
    else if(aData.indexOf("error err_code=") != -1)
    {
        if (aData.indexOf("err_code=\"TRUE\"") != -1)
        {
            emit logMessage("Error: GroovesharkLibentication error [Nick is already logged in]");
        }
        else
        {
            QDomNode error = doc.elementsByTagName("error").item(0);
            QString errText = error.attributes().namedItem("err_text").nodeValue();
            QString errCode = error.attributes().namedItem("err_code").nodeValue();
            emit logMessage(tr("Error: GroovesharkLibentication error %1 [%2]").arg(errCode, errText));
        }
    }
    else
    {
        emit logMessage("Error: GroovesharkLibentication error.");
        emit logMessage(aData);
    }
}
