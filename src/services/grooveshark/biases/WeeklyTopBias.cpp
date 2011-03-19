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

#include "WeeklyTopBias.h"

#include "core/collections/Collection.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "core/support/Debug.h"
#include "core/meta/Meta.h"
#include "core/collections/QueryMaker.h"

// #include <grooveshark/Artist>
// #include <grooveshark/ws.h>
// #include <grooveshark/XmlQuery>

#include <QByteArray>
#include <QDate>
#include <QDomDocument>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QTimeEdit>
#include <QVBoxLayout>

// class WeeklyTopBiasFactory
#include <QSignalMapper>

Dynamic::WeeklyTopBiasFactory::WeeklyTopBiasFactory()
{

}

Dynamic::WeeklyTopBiasFactory::~WeeklyTopBiasFactory()
{

}


QString
Dynamic::WeeklyTopBiasFactory::name() const
{
    return i18n( "Weekly Top Artists" );
}

QString
Dynamic::WeeklyTopBiasFactory::pluginName() const
{
    return "grooveshark_weeklytop";
}

Dynamic::CustomBiasEntry*
Dynamic::WeeklyTopBiasFactory::newCustomBiasEntry()
{
    return new Dynamic::WeeklyTopBias();
}

Dynamic::CustomBiasEntry*
Dynamic::WeeklyTopBiasFactory::newCustomBiasEntry( QDomElement e )
{
    /*
    qDebug() << "weekly top created with:" << e;
    uint from = e.firstChildElement( "from" ).attribute( "value" ).toUInt();
    uint to = e.firstChildElement( "to" ).attribute( "value" ).toUInt();
    return new Dynamic::WeeklyTopBias( from, to );
    */
    return new Dynamic::WeeklyTopBias( 666, 999 );
}

// CLASS WeeklyTopBias

Dynamic::WeeklyTopBias::WeeklyTopBias( uint from, uint to )
    : Dynamic::CustomBiasEntry()
    , m_layout( 0 )
    , m_fromEdit( 0 )
    , m_toEdit( 0 )
    , m_fromDate( from )
    , m_toDate( to )
    , m_fetching( 0 )
    , m_rangeJob( 0 )
    , m_dataJob( 0 )
{
    qDebug() << "CREATING WEEKLY TOP BIAS with dates:" << m_fromDate << m_toDate;
    QFile file( Amarok::saveLocation() + "dynamic_grooveshark_topweeklyartists.xml" );
    file.open( QIODevice::ReadOnly | QIODevice::Text );
    QTextStream in( &file );
    while( !in.atEnd() )
    {
        QString line = in.readLine();
        m_weeklyChartData.insert( line.split( "#" )[ 0 ].toUInt(), line.split( "#" )[ 1 ].split( "^" )  );
    }
    file.close();
   
    getPossibleRange();
}

Dynamic::WeeklyTopBias::~WeeklyTopBias()
{
    delete m_fetching;
}

QString
Dynamic::WeeklyTopBias::pluginName() const
{
    return "grooveshark_weeklytop";
}

QWidget*
Dynamic::WeeklyTopBias::configWidget( QWidget* parent )
{
    DEBUG_BLOCK

    QFrame * frame = new QFrame( parent );
    m_layout = new QVBoxLayout( frame );

    QLabel * label = new QLabel( i18n( "Play Top Artists From" ), frame );
    label->setWordWrap( true );
    label->setAlignment( Qt::AlignCenter );
    m_layout->addWidget( label, Qt::AlignCenter );

    m_fromEdit = new QDateTimeEdit( QDate::currentDate().addDays( -7 ) );
    m_fromEdit->setMinimumDate( QDate::currentDate().addYears( -10 ) ); // 10 years ago is minimum for now
    m_fromEdit->setMaximumDate( QDate::currentDate() );
    m_fromEdit->setCalendarPopup( true );
    if( m_fromDate > 0 )
        m_fromEdit->setDateTime( QDateTime::fromTime_t( m_fromDate ) );
    
    m_layout->addWidget( m_fromEdit );

    connect( m_fromEdit, SIGNAL( dateTimeChanged( const QDateTime& ) ), this, SLOT( fromDateChanged( const QDateTime& ) ) );

    QLabel * to = new QLabel( i18nc( "From one date to another, this text is in between", "to (will round to nearest week)" ), parent );

    to->setAlignment( Qt::AlignCenter );
    m_layout->addWidget( to, Qt::AlignCenter );

    m_toEdit =new QDateTimeEdit( QDate::currentDate().addDays( -7 ) );
    m_toEdit->setMinimumDate( QDate::currentDate().addYears( -10 ) ); // 10 years ago is minimum for now
    m_toEdit->setMaximumDate( QDate::currentDate() );
    m_toEdit->setCalendarPopup( true );
    if( m_toDate > 0 )
        m_toEdit->setDateTime( QDateTime::fromTime_t( m_toDate ) );
    
    m_layout->addWidget( m_toEdit );

    connect( m_toEdit, SIGNAL( dateTimeChanged( const QDateTime& ) ), this, SLOT( toDateChanged( const QDateTime& ) ) );

    return frame;
}

bool
Dynamic::WeeklyTopBias::trackSatisfies( const Meta::TrackPtr track )
{
    Q_UNUSED( track )
    return false;
}

double
Dynamic::WeeklyTopBias::numTracksThatSatisfy( const Meta::TrackList& tracks )
{
    Q_UNUSED( tracks )
    return 0;
}

QDomElement
Dynamic::WeeklyTopBias::xml( QDomDocument doc ) const
{
    QDomElement e = doc.createElement( "custombias" );
    e.setAttribute( "type", "weeklytop" );

    QDomElement from = doc.createElement( "from" );
    from.setAttribute( "value", QString::number( m_fromDate ) );

    QDomElement to = doc.createElement( "to" );
    to.setAttribute( "value", QString::number( m_toDate ) );

    e.appendChild( from );
    e.appendChild( to );

    return e;
}

bool
Dynamic::WeeklyTopBias::hasCollectionFilterCapability()
{
    return true;
}

Dynamic::CollectionFilterCapability*
Dynamic::WeeklyTopBias::collectionFilterCapability( double weight )
{

    qDebug() << "returning new cfb with weight:" << weight;
    return new Dynamic::WeeklyTopBiasCollectionFilterCapability( this, weight );
}

void
Dynamic::WeeklyTopBias::fromDateChanged( const QDateTime& d ) // SLOT
{
    DEBUG_BLOCK
    m_fromDate = d.toTime_t();

    update();
    emit biasChanged();
}

void
Dynamic::WeeklyTopBias::toDateChanged( const QDateTime& d ) // SLOT
{
    DEBUG_BLOCK

    m_toDate = d.toTime_t();

    update();
    emit biasChanged();
}

void
Dynamic::WeeklyTopBias::rangeJobFinished() // SLOT
{
    DEBUG_BLOCK
    if( !m_rangeJob )
        return;

    QDomDocument doc;
    if( !doc.setContent( m_rangeJob->readAll() ) )
    {
        qDebug() << "couldn't parse XML from rangeJob!";
        return;
    }

    QDomNodeList nodes = doc.elementsByTagName( "chart" );
    if( nodes.count() == 0 )
    {
        qDebug() << "USER has no history! can't do this!";
        return;
    }

    m_earliestDate = nodes.at( 0 ).attributes().namedItem( "from" ).nodeValue().toUInt();

    for( int i = 0; i < nodes.size(); i++ )
    {
        QDomNode n = nodes.at( i );
        m_weeklyCharts.append( n.attributes().namedItem( "from" ).nodeValue().toUInt() );
        m_weeklyChartsTo.append( n.attributes().namedItem( "to" ).nodeValue().toUInt() );
    }
    
    qDebug() << "got earliest date:"  << QDateTime::fromTime_t( m_earliestDate ).toString();
    if( m_fromEdit )
        m_fromEdit->setMinimumDate( QDateTime::fromTime_t( m_earliestDate ).date() );

    m_rangeJob->deleteLater();

    update();
}

void
Dynamic::WeeklyTopBias::updateReady( QString collectionId, QStringList uids )
{
    DEBUG_BLOCK
    Q_UNUSED( collectionId );

    const int protocolLength = QString( m_collection->uidUrlProtocol() + "://" ).length();

//     qDebug() << "setting cache of top UIDs for selected date: to:" << uids;
    m_trackList.clear();
    m_trackList.reserve( uids.size() );

    QByteArray uid;
    foreach( const QString &uidString, uids )
    {
        uid = uidString.mid( protocolLength ).toAscii();
        m_trackList.insert( uid );
    }

}


void
Dynamic::WeeklyTopBias::getPossibleRange()
{
    DEBUG_BLOCK

    QMap< QString, QString > params;
    params[ "method" ] = "user.getWeeklyChartList" ;
    //params[ "user" ] = grooveshark::ws::Username;

    //m_rangeJob = grooveshark::ws::get( params );

    //connect( m_rangeJob, SIGNAL( finished() ), this, SLOT( rangeJobFinished() ) );

}


void
Dynamic::WeeklyTopBias::update()
{
    DEBUG_BLOCK
    
    qDebug() << m_fromDate << m_toDate;
    if( m_fromDate >= m_toDate )
    {
        qDebug() << "Chose date limits that don't make sense! do nothing!";
        return;
    } else if( m_fromDate < m_earliestDate )
    {
        qDebug() << "User doesn't have history that goes back this far! rounding!";
        m_fromDate = m_earliestDate;
    }

    fetchWeeklyData( m_fromDate, m_toDate );
}


void Dynamic::WeeklyTopBias::fetchWeeklyData(uint from, uint to)
{
    DEBUG_BLOCK
    qDebug() << "getting top artist info from" << QDateTime::fromTime_t( from ) << "to" << QDateTime::fromTime_t( to );
    // find first weekly chart that contains dateTime
    int earliest = 0, last = 0;
    qDebug() << "number of weekly charts:" << m_weeklyCharts.size();
    for( int i = 0; i < m_weeklyCharts.size(); i++ )
    {
        if( earliest == 0 && m_weeklyCharts[ i ] > from )
        {
            earliest = m_weeklyCharts[ i - 1 ];
//             qDebug() << "chose early boundary:" << i - 1;
        }
        if( last == 0 && m_weeklyCharts[ i ] > to )
        {
//             qDebug() << "chose late boundary:" << i - 1;
            last = m_weeklyCharts[ i - 1];
        }
    }
    if( last == 0 )
        last = m_weeklyCharts[ m_weeklyCharts.size() - 1 ];

    m_currentArtistList.clear();
    m_trackList.clear();
    qDebug() << "fetching charts with these ranges:" << QDateTime::fromTime_t( earliest ) << QDateTime::fromTime_t( last );


    bool validFetch = false;
    for( int i = m_weeklyCharts.indexOf( earliest ); i < m_weeklyCharts.indexOf( last ); i++ )
    {
        validFetch = true;
        if( m_weeklyChartData.keys().contains( m_weeklyCharts[ i ] ) )
        {
            m_currentArtistList.append( m_weeklyChartData[ m_weeklyCharts[ i ] ] );
            qDebug() << "found already-saved data for week:" << m_weeklyCharts[ i ] << m_weeklyChartData[ m_weeklyCharts[ i ] ];
        } else
        {
            QMap< QString, QString > params;
            params[ "method" ] = "user.getWeeklyArtistChart";
            //params[ "user" ] = grooveshark::ws::Username;
            params[ "from" ] = QString::number( m_weeklyCharts[ i ] );
            params[ "to" ] = QString::number( m_weeklyChartsTo[ i ] );

            m_fetchQueue.enqueue( params );
        }

    }

    m_fetching = new QSignalMapper;
    connect( m_fetching, SIGNAL( mapped(QObject*) ), this, SLOT( weeklyFetch( QObject* ) ) );
    connect( this, SIGNAL( doneFetching() ), this, SLOT( updateDB() ) );
    connect( this, SIGNAL( doneFetching() ), this, SLOT( saveDataToFile() ) );

    // everything is stored, so we have all the data, finish immediately
    if( m_fetchQueue.isEmpty() && validFetch )
    {
        updateDB();
    } else
    {
        fetchNextWeeks();
    }
}

void Dynamic::WeeklyTopBias::fetchNextWeeks( int num )
{

    // fetch 5 at a time, so as to conform to grooveshark api requirements
    for(int i = 0; i < num; i++)
    {
        if( m_fetchQueue.isEmpty() )
        {
            break;
        }
//         qDebug() << "queueing up weekly fetch job!";
        //QNetworkReply* res = grooveshark::ws::get( m_fetchQueue.dequeue() );
        //connect( res, SIGNAL( finished() ), m_fetching, SLOT( map() ) );
        //m_fetching->setMapping( res, res );
    }
    
}

void Dynamic::WeeklyTopBias::weeklyFetch( QObject* reply )
{
    DEBUG_BLOCK
    if( !reply || !dynamic_cast<QNetworkReply*>( reply ) ) {
        qDebug() << "failed to get qnetwork reply in finished slot:" << reply;
        return;
    }
    QNetworkReply* r = static_cast< QNetworkReply* >( reply );

    /*
    try
    {
        grooveshark::XmlQuery lfm( r->readAll() );

//         qDebug() << "got response:" << lfm;
        QStringList artists;
        for( int i = 0; i < lfm[ "weeklyartistchart" ].children( "artist" ).size(); i++ )
        {
            if( i == 12 )
                break;
            grooveshark::XmlQuery artist = lfm[ "weeklyartistchart" ].children( "artist" ).at( i );
            artists.append( artist[ "name" ].text() );
        }
        qDebug() << "got artists:" << artists << QDomElement( lfm[ "weeklyartistchart" ] ).attribute( "from" );
        m_weeklyChartData.insert( QDomElement( lfm[ "weeklyartistchart" ] ).attribute( "from" ).toUInt(), artists );
        m_currentArtistList.append( artists );
    } catch( grooveshark::ws::ParseError& e )
    {
        qDebug() << "caught exception parsing weekly artist chart.";
    }

    reply->deleteLater();
    if( m_fetchQueue.isEmpty() )
    {
        emit doneFetching();
    } else
    {
        fetchNextWeeks( 1 );
    }
    */
}


void Dynamic::WeeklyTopBias::updateDB()
{
    m_collection = CollectionManager::instance()->primaryCollection();
    if( !m_collection )
        return;
    Collections::QueryMaker *qm = m_collection->queryMaker();

    if( !qm ) // maybe this is during startup and we don't have a QM for some reason yet
        return;

    qDebug() << "setting up query";

    qm->beginOr();
    foreach( QString artist, m_currentArtistList )
    {
        qm->beginOr();
        qDebug() << "adding artist to query:" << artist;
        qm->addFilter( Meta::valArtist, artist, true, true );
        qm->endAndOr();
    }
    qm->endAndOr();


    qm->setQueryType( Collections::QueryMaker::Custom );
    qm->addReturnValue( Meta::valUniqueId );
    qm->orderByRandom(); // as to not affect the amortized time
    qm->setAutoDelete( true );

    connect( qm, SIGNAL( newResultReady( QString, QStringList ) ),
             SLOT( updateReady( QString, QStringList ) ), Qt::DirectConnection );

    qm->run();
}


void Dynamic::WeeklyTopBias::saveDataToFile()
{
    QFile file( Amarok::saveLocation() + "dynamic_grooveshark_topweeklyartists.xml" );
    file.open( QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text );
    QTextStream out( &file );
    foreach( uint key, m_weeklyChartData.keys() )
    {
        out << key << "#" << m_weeklyChartData[ key ].join( "^" ) << endl;
    }
    file.close();

}


// Class WeeklyTopBiasCollectionFilterCapability

const QSet< QByteArray >&
Dynamic::WeeklyTopBiasCollectionFilterCapability::propertySet()
{
    return m_bias->m_trackList;
}

double
Dynamic::WeeklyTopBiasCollectionFilterCapability::weight() const
{
    return m_weight;
}


#include "WeeklyTopBias.moc"
