/***************************************************************************
                        playlistitem.cpp  -  description
                           -------------------
  begin                : Die Dez 3 2002
  copyright            : (C) 2002 by Mark Kretschmann
  email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "amarokconfig.h"
#include "app.h"
#include "metabundle.h"
#include "playlistitem.h"
#include "playlist.h"

#include <qcolor.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qstring.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kurl.h>


QColor PlaylistItem::glowText;
QColor PlaylistItem::glowBase;


// These are untranslated and used for storing/retrieving XML playlist
const QString PlaylistItem::columnName(int n) const
{
   switch(n) {
      case TrackName:
         return "TrackName";
         break;
      case Title:
         return "Title";
         break;
      case Artist:
         return "Artist";
         break;
      case Album:
         return "Album";
         break;
      case Year:
         return "Year";
         break;
      case Comment:
         return "Comment";
         break;
      case Genre:
         return "Genre";
         break;
      case Track:
         return "TrackNo";
         break;
      case Directory:
         return "Directory";
         break;
      case Length:
         return "Length";
         break;
      case Bitrate:
         return "Bitrate";
         break;
   }
   return "<ERROR>";
}

static inline QColor
mixColors( const QColor &c1, const QColor &c2, uint f1 = 1, uint f2 = 1 )
{
    const uint denominator = f1 + f2;

    return QColor( (c1.red()*f1   + c2.red()*f2)   / denominator,
                   (c1.green()*f1 + c2.green()*f2) / denominator,
                   (c1.blue()*f1  + c2.blue()*f2)  / denominator );
}

static inline QColor
changeContrast( const QColor &c, int contrast )
{
    //you get contrast by changing the saturation and value parameters

    int h,s,v; c.getHsv( &h, &s, &v );

    s += contrast;

    if( s > 255 )    { int d = 255 - s; v += d; s = 255; }
    else if( s < 0 ) { v += s; s = 0; }

    //if v is out of range then leave it, no additional contrast is possible

    return QColor( h,s,v, QColor::Hsv );
}



//statics
QString PlaylistItem::stringStore[STRING_STORE_SIZE];



PlaylistItem::PlaylistItem( Playlist* parent, QListViewItem *lvi, const KURL &u, const QString &title, const int length )
  : KListViewItem( parent, lvi, trackName( u ) )
#ifdef CORRUPT_FILE
  , m_playing( false )
  , corruptFile( FALSE ) //our friend threadweaver will take care of this flag
#endif
  , m_url( u )
{
    setDragEnabled( true );

    KListViewItem::setText( Title, title );
    setText( Directory, u.directory().section( '/', -1 ) ); //try for stringStore
    setText( Length, MetaBundle::prettyLength( length ) );
}


PlaylistItem::PlaylistItem( Playlist* parent, QListViewItem *lvi, const KURL &u, const QDomNode &n )
      : KListViewItem( parent, lvi, trackName( u ) )
      , m_url( u )
{
    setDragEnabled( true );

    const uint ncol = parent->columns();

    //NOTE we use base versions to speed this up (this function is called 100s of times during startup)
    for( uint x = 1; x < ncol; ++x )
    {
        const QString text = n.namedItem( columnName( x ) ).toElement().text();

        //FIXME this is duplication of setText()
        //TODO  it would be neat to have all store columns adjacent and at top end so you can use
        //      a simple bit of logic to discern which ones to store
        //FIXME use the MetaBundle implicitly shared bitrate and track # strings
        switch( x ) {
        case Artist:
        case Album:
        case Genre:
        case Year:
        case Directory:
            KListViewItem::setText( x, attemptStore( text ) );
            continue;
        default:
            KListViewItem::setText( x, text );
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////////////

#include <taglib/fileref.h>
#include <taglib/audioproperties.h>

MetaBundle PlaylistItem::metaBundle()
{
    //TODO this meta prop reading causes ogg files to skip, so we need to do it a few seconds before the
    //ogg file starts playing or something

    //Do this everytime to save cost of storing int for length/samplerate/bitrate
    //This function isn't called often (on play request), but playlists can contain
    //thousands of items. So favor saving memory over CPU.

    if ( m_url.isLocalFile() ) {
        TagLib::FileRef f( m_url.path().local8Bit(), true, TagLib::AudioProperties::Accurate );
        //FIXME hold a small cache of metabundles?
        //then return by reference
        MetaBundle bundle( this, f.isNull() ? 0 : f.audioProperties() );
        //just set it as we just did an accurate pass
        setText( Length,  bundle.prettyLength()  );
        setText( Bitrate, bundle.prettyBitrate() );

        return bundle;

    } else
        return MetaBundle( this, 0 );
}


QString PlaylistItem::text( int column ) const
{
    //if trackname (column 0) is hidden, then show trackname text in title column if there
    //is no text set for the title (column 1)

    if( column == Title && listView()->columnWidth( TrackName ) == 0 && KListViewItem::text( Title ).isEmpty() )
    {
            return KListViewItem::text( TrackName );
    }

    return KListViewItem::text( column );
}


QString
PlaylistItem::seconds() const
{
    QString length = exactText( Length );

    if( length == "?" ) return QString();
    if( length == "-" ) length += '1';
    else if( !length.isEmpty() )
    {
        int m = length.section( ':', 0, 0 ).toInt();
        int s = length.section( ':', 1, 1 ).toInt();

        length.setNum( m * 60 + s );
    }

    return length;
}

void PlaylistItem::setText( const MetaBundle &bundle )
{
    setText( Title,   bundle.prettyTitle() );
    setText( Artist,  bundle.artist() );
    setText( Album,   bundle.album() );
    setText( Year,    bundle.year() );
    setText( Comment, bundle.comment() );
    setText( Genre,   bundle.genre() );
    setText( Track,   bundle.track() );
    setText( Length,  bundle.prettyLength() );
    setText( Bitrate, bundle.prettyBitrate() );
}


void PlaylistItem::setText( int column, const QString &newText )
{
    //NOTE prettyBitrate() is special and the returned string should not be modified
    //     as it is implicately shared for the common values in class MetaBundle
    //NOTE track() may also be special

    switch( column ) {
    case Artist:
    case Album:
    case Genre:
    case Year: //TODO check this doesn't hog the store
    case Directory:

        //these are good candidates for the stringStore
        //NOTE title is not a good candidate, it probably will never repeat in the playlist
        KListViewItem::setText( column, attemptStore( newText ) );
        break;

    case Length:
        //TODO consider making this a dynamically generated string
        KListViewItem::setText( Length, newText.isEmpty() ? newText : newText + ' ' ); //padding makes it neater
        break;

//     case 1:
//     case 9:
//     case 10:
//         if( newText.isEmpty() )
//         {
//             //FIXME this is an awful hack, do something about it
//
//             //don't overwrite old text with nothing
//             //we do this because there are several stages to setting text when items are inserted into the
//             //playlist, and not all of them have text set.
//             //we only need to do this for columns 1, 9 and 10 currently
//
//             //FIXME removing a tag with inline edit doesn't get updated here, but
//             //      you've hacked TagWriter so it sets a space as the new text
//             //FIXME that needs fixing because it means the scrolling title has a space! dang.
//
//             //NOTE if you don't setText() it crashes amaroK!
//             KListViewItem::setText( column, KListViewItem::text( column ) );
//
//             break;
//         }
//
//         //else FALL THROUGH

    default:
        KListViewItem::setText( column, newText );
        break;
    }

    //FIXME this can be done multiple times, eg setText(MetaBundle&)
    switch( column ) {
    case TrackName:
    case Title:
    case Artist:
    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////////////////

int
PlaylistItem::compare( QListViewItem *i, int col, bool ascending ) const
{
    float a, b;

    switch( col )  //we cannot sort numbers lexically, so we must special case those columns
    {
        case Track:
        case Year:
            a =    text( col ).toFloat();
            b = i->text( col ).toFloat();
            break;

        case Length:
            a =    text( Length ).replace( ':', '.' ).toFloat();
            b = i->text( Length ).replace( ':', '.' ).toFloat();
            break;

        case Bitrate:
            a =    text( Bitrate ).left( 3 ).toFloat(); //should work for 10 through 999 kbps
            b = i->text( Bitrate ).left( 3 ).toFloat(); //made this change due to setText space paddings
            break;

        case Artist:
            if( text( Artist ) == i->text( Artist ) ) //if same artist, try to sort by album
            {
                return this->compare( i, Album, ascending );
            }
            else goto lexical;

        case Album:
            if( text( Album ) == i->text( Album ) ) //if same album, try to sort by track
            {
                return this->compare( i, Track, true ); //only sort in ascending order //FIXME don't work
            }

            //else FALL_THROUGH..

        lexical:
        default:
            //is an ordinary string -> sort lexically
            return KListViewItem::compare( i, col, ascending );
    }

    if ( a > b ) return +1;
    if ( a < b ) return -1;

    return 0;    //a == b
}

void PlaylistItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
    //TODO add spacing on either side of items
    //p->translate( 2, 0 ); width -= 3;

    //FIXME this was crashing stuff when you were dropping remote playlists
    //TODO  anyway it sucks, load audio props simultaneously to other tags
    //TODO  don't read audioproperties if their columns aren't shown and re-read tags if those columns are then shown
    //if( column == Length && text( Length ).isEmpty() ) listView()->readAudioProperties( this );

    //determine first visible column
    QHeader* header = listView()->header();
    int firstCol;
    for ( firstCol = 0; firstCol < header->count(); firstCol++ )
        if ( header->sectionSize( header->mapToSection( firstCol ) ) )
            break;
    //convert to logical column
    firstCol = header->mapToSection( firstCol );

    //Allocate buffer pixmap, for flicker-free drawing
    QPixmap* buffer = new QPixmap( width, height() );
    QPainter painterBuf( buffer, true );
    painterBuf.setFont( p->font() );

    int  playNext = listView()->m_nextTracks.findRef( this ) + 1;
    
    //HACK, but I don't know how to restore the height otherwise; calling invalidateHeight inside of 
    //paintCell() slows things down extremely
    if ( height() != listView()->fontMetrics().height() * 2 )
        m_cachedHeight = height();
    
    if( this == listView()->currentTrack() )
    {
        if ( m_playing && column == firstCol )
            //display "Play" icon
            setPixmap( column, SmallIcon( "artsbuilderexecute" ) );
        else
            //hide "Play" icon
            setPixmap( column, 0 );
                    
        setHeight( listView()->fontMetrics().height() * 2 );
        QColorGroup glowCg = cg; //shallow copy

        glowCg.setColor( QColorGroup::Base, glowBase );
        glowCg.setColor( QColorGroup::Text, glowText );

        //KListViewItem enforces alternate color, so we use QListViewItem
        QListViewItem::paintCell( &painterBuf, glowCg, column, width, align );
    }
    else {
        //hide "Play" icon
        setPixmap( column, 0 );
        setHeight( m_cachedHeight );
        KListViewItem::paintCell( &painterBuf, cg, column, width, align );
    }
        
    //figure out if we are in the actual physical first column
    if( playNext && column == firstCol )
    {
        QString str = QString::number( playNext );

        //draw the symbol's outline
              uint fw = p->fontMetrics().width( str ) + 2;
        const uint w  = 16; //keep this even
        const uint h  = height() - 2;

        painterBuf.setBrush( cg.highlight() );
        painterBuf.setPen( cg.highlight().dark() ); //TODO blend with background color
        painterBuf.drawEllipse( width - fw - w/2, 1, w, h );
        painterBuf.drawRect( width - fw, 1, fw, h );
        painterBuf.setPen( cg.highlight() );
        painterBuf.drawLine( width - fw, 2, width - fw, h - 1 );

        //draw the shadowed inner text
        //NOTE we can't set an arbituary font size or family, these settings are already optional
        //and user defaults should also take presidence if no playlist font has been selected
        //const QFont smallFont( "Arial", (playNext > 9) ? 9 : 12 );
        //p->setFont( smallFont );
        //TODO the shadow is hard to do well when using a dark font color
        //TODO it also looks cluttered for small font sizes
        //p->setPen( cg.highlightedText().dark() );
        //p->drawText( width - w + 2, 3, w, h-1, Qt::AlignCenter, str );
        fw += 2; //add some more padding
        painterBuf.setPen( cg.highlightedText() );
        painterBuf.drawText( width - fw, 2, fw, h-1, Qt::AlignCenter, str );
    }

    if( !isSelected() )
    {
        painterBuf.setPen( QPen( cg.dark(), 0, Qt::SolidLine ) );
        painterBuf.drawLine( width - 1, 0, width - 1, height() - 1 );
    }

    painterBuf.end();
    p->drawPixmap( 0, 0, *buffer );
    delete buffer;
}


const QString &PlaylistItem::attemptStore( const QString &candidate ) //static
{
    //principal is to cause collisions at reasonable rate to reduce memory
    //consumption while not using such a big store that it is mostly filled with empty QStrings
    //because collisions are so rare

    if( candidate.isEmpty() ) return candidate; //nothing to try to share

    const uchar hash = candidate[0].unicode() % STRING_STORE_SIZE;


    if( stringStore[hash] != candidate ) //then replace
    {
        stringStore[hash] = candidate;
    }

    return stringStore[hash];
}
