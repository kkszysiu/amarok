/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
 * Copyright (c) 2008 Leo Franchi <lfranchi@kde.org>                                    *
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

#ifndef GROOVESHARKSERVICE_H
#define GROOVESHARKSERVICE_H


#include "../ServiceBase.h"

#include <KLineEdit>
#include <QLabel>
class ScrobblerAdapter;
class GroovesharkService;

namespace Collections {
    class GroovesharkServiceCollection;
}

class QNetworkReply;

class KHBox;

class QComboBox;

namespace The
{
    GroovesharkService *groovesharkService();
}

namespace Dynamic {
    class GroovesharkBiasFactory;
    class WeeklyTopBiasFactory;
}

class GroovesharkServiceFactory : public ServiceFactory
{
    Q_OBJECT

public:
    GroovesharkServiceFactory( QObject *parent, const QVariantList &args );
    virtual ~GroovesharkServiceFactory() {}

    virtual void init();
    virtual QString name();
    virtual KConfigGroup config();

    virtual bool possiblyContainsTrack( const KUrl &url ) const { return url.protocol() == "grooveshark"; }

private slots:
    void slotCreateGroovesharkService();
    void slotRemoveGroovesharkService();

private:
    ServiceBase* createGroovesharkService();
};

class GroovesharkService : public ServiceBase
{
    Q_OBJECT

public:
    GroovesharkService( GroovesharkServiceFactory* parent, const QString &name, const QString &username, QString password, const QString &sessionKey, bool scrobble, bool fetchSimilar );
    virtual ~GroovesharkService();

    virtual void polish();

    ScrobblerAdapter *scrobbler() { return m_scrobbler; }

    virtual Collections::Collection * collection();

    void love( Meta::TrackPtr track );

private slots:
    void love();
    void skip();
    void ban();

    void playCustomStation();
    void updateEditHint( int index );

    void onAuthenticated();
    void onGetUserInfo();
    void onAvatarDownloaded( const QString& username, QPixmap avatar );

private:
    void init();

    bool m_inited;
    bool m_scrobble;
    ScrobblerAdapter *m_scrobbler;
    Collections::GroovesharkServiceCollection *m_collection;

    void playGroovesharkStation( const KUrl &url );
    void updateProfileInfo();

    bool m_polished;
    QWidget *m_profileBox;
    QLabel *m_avatarLabel;
    QLabel *m_profile;
    QLabel *m_userinfo;

    QComboBox *m_globalComboBox;

    KLineEdit * m_customStationEdit;
    QPushButton * m_customStationButton;
    QComboBox * m_customStationCombo;

    QString m_userName;
    QString m_sessionKey;
    QString m_station;
    QString m_age;
    QString m_gender;
    QString m_country;
    QString m_playcount;
    QPixmap m_avatar;
    bool m_subscriber;

    char *m_userNameArray;
    char *m_sessionKeyArray;

    Dynamic::GroovesharkBiasFactory* m_groovesharkBiasFactory;
    Dynamic::WeeklyTopBiasFactory* m_weeklyTopBiasFactory;

    QMap< QString, QNetworkReply* > m_jobs;
    static GroovesharkService *ms_service;

    friend GroovesharkService *The::groovesharkService();
};

#endif // GROOVESHARKSERVICE_H
