/****************************************************************************************
 * Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>                                    *
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

#ifndef GROOVESHARK_BIAS_H
#define GROOVESHARK_BIAS_H

#include "CustomBiasEntry.h"
#include "CustomBiasEntryFactory.h"

#include <QNetworkReply>

/**
 *  This is a bias which adds the suggested songs to the playlist.
 *
 */

class KComboBox;

namespace Collections
{
    class Collection;
    class QueryMaker;
}

namespace Dynamic
{

class GroovesharkBiasFactory : public CustomBiasEntryFactory
{
    public:
        GroovesharkBiasFactory();
        ~GroovesharkBiasFactory();

        virtual QString name() const;
        virtual QString pluginName() const;
        virtual CustomBiasEntry* newCustomBiasEntry();
        virtual CustomBiasEntry* newCustomBiasEntry( QDomElement e );
};

// this order of inheritance is a bit screwy, but moc wants the QObject-derived class to be first always
class GroovesharkBias : public CustomBiasEntry
{
    Q_OBJECT
public:
    GroovesharkBias( bool similarArtists );
    virtual ~GroovesharkBias();

    // reimplemented from CustomBiasEntry
    virtual QString pluginName() const;
    virtual QWidget* configWidget ( QWidget* parent );

    virtual bool trackSatisfies ( const Meta::TrackPtr track );
    virtual double numTracksThatSatisfy ( const Meta::TrackList& tracks );

    virtual QDomElement xml( QDomDocument doc ) const;

    virtual bool hasCollectionFilterCapability();
    virtual CollectionFilterCapability* collectionFilterCapability( double weight );

    void update();

Q_SIGNALS:
    void doneFetching();

private Q_SLOTS:
    void artistQueryDone();
    void trackQueryDone();
    void updateBias();
    void artistUpdateReady ( QString collectionId, QStringList );
    void trackUpdateReady ( QString collectionId, QStringList );
    void updateFinished();
    void collectionUpdated();
    void activated( int index );
    void saveDataToFile();

private:
    void addData( QString collectionId, QStringList uids, const QString& index, QMap< QString, QSet< QByteArray > >& storage );
    void loadFromFile();
    bool m_similarArtists;
    QString m_currentArtist;
    QString m_currentTrack;
    QNetworkReply* m_artistQuery;
    QNetworkReply* m_trackQuery;
    Collections::QueryMaker* m_qm; // stored so it can be refreshed
    // if the collection changes
    Collections::Collection* m_collection; // null => all queryable collections
    bool m_needsUpdating;
    QMutex m_mutex;

    KComboBox* m_combo;
    QMap< QString, QSet< QByteArray > > m_savedArtists; // populated as queries come in
    QMap< QString, QSet< QByteArray > > m_savedTracks; // populated as queries come in
    // we do some caching here so multiple
    // queries of the same artist are cheap
    friend class GroovesharkBiasCollectionFilterCapability; // so it can report the property and weight
};


class GroovesharkBiasCollectionFilterCapability : public Dynamic::CollectionFilterCapability
{
public:
    GroovesharkBiasCollectionFilterCapability ( GroovesharkBias* bias, double weight ) : m_bias ( bias ), m_weight( weight ) {}

    // re-implemented
    virtual const QSet<QByteArray>& propertySet();
    virtual double weight() const;


private:
    GroovesharkBias* m_bias;
    double m_weight;

};

}

#endif
