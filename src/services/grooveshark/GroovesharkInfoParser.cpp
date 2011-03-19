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

#include "GroovesharkInfoParser.h"

#include "core/support/Debug.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"
#include "GroovesharkConfig.h"

#include <KLocale>


using namespace Meta;

void GroovesharkInfoParser::getInfo(ArtistPtr artist)
{

    showLoading( i18n( "Loading artist info..." ) );

    GroovesharkArtist * groovesharkArtist = dynamic_cast<GroovesharkArtist *>( artist.data() );
    if ( groovesharkArtist == 0) return;

    debug() << "GroovesharkInfoParser: getInfo about artist";

    // first get the entire artist html page
   /* QString tempFile;
    QString orgHtml;*/

    m_infoDownloadJob = KIO::storedGet( groovesharkArtist->groovesharkUrl(), KIO::Reload, KIO::HideProgressInfo );
    Amarok::Components::logger()->newProgressOperation( m_infoDownloadJob, i18n( "Fetching %1 Artist Info", groovesharkArtist->prettyName() ) );
    connect( m_infoDownloadJob, SIGNAL(result(KJob *)), SLOT( artistInfoDownloadComplete( KJob*) ) );

}


void GroovesharkInfoParser::getInfo(AlbumPtr album)
{

    showLoading( i18n( "Loading album info..." ) );
    
    GroovesharkAlbum * groovesharkAlbum = dynamic_cast<GroovesharkAlbum *>( album.data() );

    const QString artistName = album->albumArtist()->name();

    QString infoHtml = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" "
                       "CONTENT=\"text/html; charset=utf-8\"></HEAD><BODY>";

    infoHtml += generateHomeLink();
    infoHtml += "<div align=\"center\"><strong>";
    infoHtml += artistName;
    infoHtml += "</strong><br><em>";
    infoHtml += groovesharkAlbum->name();
    infoHtml += "</em><br><br>";
    infoHtml += "<img src=\"" + groovesharkAlbum->coverUrl() +
                "\" align=\"middle\" border=\"1\">";

    // Disable Genre line in Grooveshark applet since, well, it doesn't actually put a genre there...
    // Nikolaj, FYI: either the thumbnails aren't working, or they aren't getting through the proxy here.  That would be odd, however, as the database and
    // all HTML are coming through the proxy
    //infoHtml += "<br><br>" + i18n( "Genre: ");// + groovesharkAlbum->
    infoHtml += "<br>" + i18n( "Release Year: ") + QString::number( groovesharkAlbum->launchYear() );

    if ( !groovesharkAlbum->description().isEmpty() ) {

        //debug() << "GroovesharkInfoParser: Writing description: '" << album->getDescription() << "'";
        infoHtml += "<br><br><b>" + i18n( "Description:" ) + "</b><br><p align=\"left\" >" + groovesharkAlbum->description();

    }

    infoHtml += "</p><br><br>" + i18n( "From Grooveshark.com" ) + "</div>";
    infoHtml += "</BODY></HTML>";

    emit ( info( infoHtml ) );
}

void GroovesharkInfoParser::getInfo(TrackPtr track)
{
    Q_UNUSED( track );
    return;
}




void
GroovesharkInfoParser::artistInfoDownloadComplete( KJob *downLoadJob )
{

    if ( !downLoadJob->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_infoDownloadJob )
        return ; //not the right job, so let's ignore it



    QString infoString = ( (KIO::StoredTransferJob* ) downLoadJob )->data();
    //debug() << "GroovesharkInfoParser: Artist info downloaded: " << infoString;
    infoString = extractArtistInfo( infoString );

    //debug() << "html: " << trimmedInfo;

    emit ( info( infoString ) );

}

QString
GroovesharkInfoParser::extractArtistInfo( const QString &artistPage )
{
    QString trimmedHtml;


    int sectionStart = artistPage.indexOf( "<!-- ARTISTBODY -->" );
    int sectionEnd = artistPage.indexOf( "<!-- /ARTISTBODY -->", sectionStart );

    trimmedHtml = artistPage.mid( sectionStart, sectionEnd - sectionStart );

    int buyStartIndex = trimmedHtml.indexOf( "<!-- PURCHASE -->" );
    int buyEndIndex;

    //we are going to integrate the buying of music (I hope) so remove these links

    while ( buyStartIndex != -1 )
    {
        buyEndIndex = trimmedHtml.indexOf( "<!-- /PURCHASE -->", buyStartIndex ) + 18;
        trimmedHtml.remove( buyStartIndex, buyEndIndex - buyStartIndex );
        buyStartIndex = trimmedHtml.indexOf( "<!-- PURCHASE -->", buyStartIndex );
    }


    QString infoHtml = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" "
                       "CONTENT=\"text/html; charset=iso-8859-1\"></HEAD><BODY>";
    infoHtml += generateHomeLink();
    infoHtml += trimmedHtml;
    infoHtml += "</BODY></HTML>";


    return infoHtml;
}

void GroovesharkInfoParser::getFrontPage()
{

    if( !m_cachedFrontpage.isEmpty() )
    {
        emit ( info( m_cachedFrontpage ) );
        return;
    }

    showLoading( i18n( "Loading Grooveshark.com frontpage..." ) );
    
    m_pageDownloadJob = KIO::storedGet( KUrl( "http://grooveshark.com/amarok_frontpage.html" ), KIO::Reload, KIO::HideProgressInfo );
    Amarok::Components::logger()->newProgressOperation( m_pageDownloadJob, i18n( "Fetching Grooveshark.com front page" ) );
    connect( m_pageDownloadJob, SIGNAL(result( KJob * ) ), SLOT( frontpageDownloadComplete( KJob*) ) );
}

void GroovesharkInfoParser::getFavoritesPage()
{
    DEBUG_BLOCK

    GroovesharkConfig config;

    if ( !config.isMember() )
        return;
    
    showLoading( i18n( "Loading your Grooveshark.com favorites page..." ) );

    QString type;
    if( config.membershipType() == GroovesharkConfig::STREAM )
        type = "stream";
    else
         type = "download";
    
    QString user = config.username();
    QString password = config.password();

    QString url = "http://" + user + ":" + password + "@" + type.toLower() + ".grooveshark.com/member/amarok_favorites.php";
   
    debug() << "loading url: " << url;

    m_pageDownloadJob = KIO::storedGet( KUrl( url ), KIO::Reload, KIO::HideProgressInfo );
    Amarok::Components::logger()->newProgressOperation( m_pageDownloadJob, i18n( "Loading your Grooveshark.com favorites page..." ) );
    connect( m_pageDownloadJob, SIGNAL(result(KJob *)), SLOT( userPageDownloadComplete( KJob*) ) );
}

void GroovesharkInfoParser::getRecommendationsPage()
{
    DEBUG_BLOCK

    GroovesharkConfig config;

    if ( !config.isMember() )
        return;

    showLoading( i18n( "Loading your personal Grooveshark.com recommendations page..." ) );

    QString type;
    if( config.membershipType() == GroovesharkConfig::STREAM )
        type = "stream";
    else
         type = "download";
    
    QString user = config.username();
    QString password = config.password();

    QString url = "http://" + user + ":" + password + "@" + type.toLower() + ".grooveshark.com/member/amarok_recommendations.php";

    debug() << "loading url: " << url;

    m_pageDownloadJob = KIO::storedGet( KUrl( url ), KIO::Reload, KIO::HideProgressInfo );
    Amarok::Components::logger()->newProgressOperation( m_pageDownloadJob, i18n( "Loading your personal Grooveshark.com recommendations page..." ) );
    connect( m_pageDownloadJob, SIGNAL(result(KJob *)), SLOT( userPageDownloadComplete( KJob*) ) );
    
}

void GroovesharkInfoParser::frontpageDownloadComplete( KJob * downLoadJob )
{
    DEBUG_BLOCK
    if ( !downLoadJob->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_pageDownloadJob )
        return ; //not the right job, so let's ignore it

    QString infoString = ((KIO::StoredTransferJob* )downLoadJob)->data();

    //insert menu
    GroovesharkConfig config;
    if( config.isMember() )
        infoString.replace( "<!--MENU_TOKEN-->", generateMemberMenu() );

    //insert fancy amarok url links to the artists
    infoString = createArtistLinks( infoString );

    if( m_cachedFrontpage.isEmpty() )
        m_cachedFrontpage = infoString;

    emit ( info( infoString ) );
}

void GroovesharkInfoParser::userPageDownloadComplete( KJob * downLoadJob )
{
    DEBUG_BLOCK
    if ( !downLoadJob->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_pageDownloadJob )
        return ; //not the right job, so let's ignore it



    QString infoString = ((KIO::StoredTransferJob* )downLoadJob)->data();

    //insert menu
    GroovesharkConfig config;
    if( config.isMember() )
        infoString.replace( "<!--MENU_TOKEN-->", generateMemberMenu() );

    //make sure that any pages that use the old command name "service_grooveshark" replaces it with "service-grooveshark"
    infoString.replace( "service_grooveshark", "service-grooveshark" );

    emit ( info( infoString ) );
}


QString GroovesharkInfoParser::generateMemberMenu()
{
    QString homeUrl = "amarok://service-grooveshark?command=show_home";
    QString favoritesUrl = "amarok://service-grooveshark?command=show_favorites";
    QString recommendationsUrl = "amarok://service-grooveshark?command=show_recommendations";

    QString menu = "<div align='right'>"
                       "[<a href='" + homeUrl + "' >Home</a>]&nbsp;"
                       "[<a href='" + favoritesUrl + "' >Favorites</a>]&nbsp;"
                       "[<a href='" + recommendationsUrl + "' >Recommendations</a>]&nbsp;"
                    "</div>";

    return menu;
}

QString
GroovesharkInfoParser::generateHomeLink()
{
    QString homeUrl = "amarok://service-grooveshark?command=show_home";
    QString link = "<div align='right'>"
                    "[<a href='" + homeUrl + "' >Home</a>]&nbsp;"
                   "</div>";

    return link;
}

QString
GroovesharkInfoParser::createArtistLinks( const QString &page )
{
    DEBUG_BLOCK
    //the artist name is wrapped in <!--ARTIST_TOKEN-->artist<!--/ARTIST_TOKEN-->

    QString returnPage = page;

    int startTokenLength = QString( "<!--ARTIST_TOKEN-->" ).length();

    int offset = 0;
    int startTokenIndex = page.indexOf( "<!--ARTIST_TOKEN-->", offset );
    int endTokenIndex = 0;

    while( startTokenIndex != -1 )
    {
        endTokenIndex = page.indexOf( "<!--/ARTIST_TOKEN-->", startTokenIndex );
        if( endTokenIndex == -1 )
            break; //bail out

        offset = endTokenIndex;

        //get the artist namespace

        int artistLength = endTokenIndex - ( startTokenIndex + startTokenLength );
        QString artist = page.mid( startTokenIndex + startTokenLength, artistLength );

        debug() << "got artist " << artist;

        //replace in the artist amarok url

        QString replaceString = "<!--ARTIST_TOKEN-->" + artist + "<!--/ARTIST_TOKEN-->";
        QString artistLink = "<a href='amarok://navigate/internet/Grooveshark.com?filter=artist:%22" + artist + "%22&levels=artist-album'>" + artist + "</a>";

        debug() << "replacing " <<  replaceString << " with " << artistLink;
        returnPage = returnPage.replace( replaceString, artistLink );

        startTokenIndex = page.indexOf( "<!--ARTIST_TOKEN-->", offset );
    }

    return returnPage;
}


#include "GroovesharkInfoParser.moc"

