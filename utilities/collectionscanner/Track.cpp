/***************************************************************************
 *   Copyright (C) 2003-2005 Max Howell <max.howell@methylblue.com>        *
 *             (C) 2003-2010 Mark Kretschmann <kretschmann@kde.org>        *
 *             (C) 2005-2007 Alexandre Oliveira <aleprj@gmail.com>         *
 *             (C) 2008 Dan Meltzer <parallelgrapefruit@gmail.com>         *
 *             (C) 2008-2009 Jeff Mitchell <mitchell@kde.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "Track.h"

#include "MetaTagLib.h"
#include "MetaReplayGain.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

bool CollectionScanner::Track::s_useCharsetDetector = false;


CollectionScanner::Track::Track( const QString &path, CollectionScanner::Directory* directory )
   : m_valid( true )
   , m_directory( directory )
   , m_filetype( Amarok::Unknown )
   , m_compilation( false )
   , m_noCompilation( false )
   , m_hasCover( false )
   , m_year( -1 )
   , m_disc( -1 )
   , m_track( -1 )
   , m_bpm( -1.0 )
   , m_bitrate( -1 )
   , m_length( -1.0 )
   , m_samplerate( -1 )
   , m_filesize( -1 )

   , m_trackGain( -1.0 )
   , m_trackPeakGain( -1.0 )
   , m_albumGain( -1.0 )
   , m_albumPeakGain( -1.0 )

   , m_rating( -1.0 )
   , m_score( -1.0 )
   , m_playcount( -1.0 )
{
    // for the unit test.
    // in a debug build a file called "crash_amarok_here.ogg" will crash the collection
    // scanner
    if( path.contains("crash_amarok_here.ogg") )
    {
        qDebug() << "Crashing at"<<path;
        Q_ASSERT( false );
    }

    Meta::FieldHash values = Meta::Tag::readTags( path, s_useCharsetDetector );

    m_valid = !values.empty();
    if( values.contains(Meta::valUniqueId) )
        m_uniqueid = values.value(Meta::valUniqueId).toString();
    m_path = path;
    m_rpath = QDir::current().relativeFilePath( path );
    if( values.contains(Meta::valFormat) )
        m_filetype = Amarok::FileType(values.value(Meta::valFormat).toInt());
    if( values.contains(Meta::valTitle) )
        m_title = values.value(Meta::valTitle).toString();
    if( values.contains(Meta::valArtist) )
        m_artist = values.value(Meta::valArtist).toString();
    if( values.contains(Meta::valAlbum) )
        m_album = values.value(Meta::valAlbum).toString();
    if( values.contains( Meta::valAlbumArtist ) )
        m_albumArtist = values.value( Meta::valAlbumArtist ).toString();
    if( values.contains(Meta::valCompilation) )
    {
        m_compilation = values.value(Meta::valCompilation).toBool();
        m_noCompilation = !values.value(Meta::valCompilation).toBool();
    }
    if( values.contains(Meta::valHasCover) )
        m_hasCover = values.value(Meta::valHasCover).toBool();
    if( values.contains(Meta::valComment) )
        m_comment = values.value(Meta::valComment).toString();
    if( values.contains(Meta::valGenre) )
        m_genre = values.value(Meta::valGenre).toString();
    if( values.contains(Meta::valYear) )
        m_year = values.value(Meta::valYear).toInt();
    if( values.contains(Meta::valDiscNr) )
        m_disc = values.value(Meta::valDiscNr).toInt();
    if( values.contains(Meta::valTrackNr) )
        m_track = values.value(Meta::valTrackNr).toInt();
    if( values.contains(Meta::valBpm) )
        m_bpm = values.value(Meta::valBpm).toReal();
    if( values.contains(Meta::valBitrate) )
        m_bitrate = values.value(Meta::valBitrate).toInt();
    if( values.contains(Meta::valLength) )
        m_length = values.value(Meta::valLength).toLongLong();
    if( values.contains(Meta::valSamplerate) )
        m_samplerate = values.value(Meta::valSamplerate).toInt();
    if( values.contains(Meta::valFilesize) )
        m_filesize = values.value(Meta::valFilesize).toLongLong();
    if( values.contains(Meta::valModified) )
        m_modified = values.value(Meta::valModified).toDateTime();

    if( values.contains(Meta::valTrackGain) )
        m_trackGain = values.value(Meta::valTrackGain).toReal();
    if( values.contains(Meta::valTrackGainPeak) )
        m_trackPeakGain = values.value(Meta::valTrackGainPeak).toReal();
    if( values.contains(Meta::valAlbumGain) )
        m_albumGain = values.value(Meta::valAlbumGain).toReal();
    if( values.contains(Meta::valAlbumGainPeak) )
        m_albumPeakGain = values.value(Meta::valAlbumGainPeak).toReal();
    while( m_uniqueid.startsWith('/') )
        m_uniqueid = m_uniqueid.mid(1);

    if( values.contains(Meta::valComposer) )
        m_composer = values.value(Meta::valComposer).toString();

    if( values.contains(Meta::valRating) )
        m_rating = values.value(Meta::valRating).toReal();
    if( values.contains(Meta::valScore) )
        m_score = values.value(Meta::valScore).toReal();
    if( values.contains(Meta::valPlaycount) )
        m_playcount = values.value(Meta::valPlaycount).toReal();
}

CollectionScanner::Track::Track( QXmlStreamReader *reader, CollectionScanner::Directory* directory )
   : m_valid( true )
   , m_directory( directory )
   , m_filetype( Amarok::Unknown )
   , m_compilation( false )
   , m_noCompilation( false )
   , m_hasCover( false )
   , m_year( -1 )
   , m_disc( -1 )
   , m_track( -1 )
   , m_bpm( -1 )
   , m_bitrate( -1 )
   , m_length( -1 )
   , m_samplerate( -1 )
   , m_filesize( -1 )

   , m_trackGain( -1 )
   , m_trackPeakGain( -1 )
   , m_albumGain( -1 )
   , m_albumPeakGain( -1 )

   , m_rating( -1 )
   , m_score( -1 )
   , m_playcount( -1 )
{
    // improve scanner with skipCurrentElement as soon as Amarok requires Qt 4.6
    while (!reader->atEnd()) {
        reader->readNext();

        if( reader->isStartElement() )
        {
            QStringRef name = reader->name();
            if( name == QLatin1String("uniqueid") )
                m_uniqueid = reader->readElementText(QXmlStreamReader::SkipChildElements);
            else if( name == QLatin1String("path") )
                m_path = reader->readElementText(QXmlStreamReader::SkipChildElements);
            else if( name == QLatin1String("rpath") )
                m_rpath = reader->readElementText(QXmlStreamReader::SkipChildElements);

            else if( name == QLatin1String("filetype") )
                m_filetype = (Amarok::FileType)reader->readElementText(QXmlStreamReader::SkipChildElements).toInt();
            else if( name == QLatin1String("title") )
                m_title = reader->readElementText(QXmlStreamReader::SkipChildElements);
            else if( name == QLatin1String("artist") )
                m_artist = reader->readElementText(QXmlStreamReader::SkipChildElements);
            else if( name == QLatin1String("albumArtist") )
                m_albumArtist = reader->readElementText(QXmlStreamReader::SkipChildElements);
            else if( name == QLatin1String("album") )
                m_album = reader->readElementText();
            else if( name == QLatin1String("compilation") )
            {
                m_compilation = true;
                reader->skipCurrentElement();
            }
            else if( name == QLatin1String("noCompilation") )
            {
                m_noCompilation = true;
                reader->skipCurrentElement();
            }
            else if( name == QLatin1String("hasCover") )
            {
                m_hasCover = true;
                reader->skipCurrentElement();
            }
            else if( name == QLatin1String("comment") )
                m_comment = reader->readElementText(QXmlStreamReader::SkipChildElements);
            else if( name == QLatin1String("genre") )
                m_genre = reader->readElementText(QXmlStreamReader::SkipChildElements);
            else if( name == QLatin1String("year") )
                m_year = reader->readElementText(QXmlStreamReader::SkipChildElements).toInt();
            else if( name == QLatin1String("disc") )
                m_disc = reader->readElementText(QXmlStreamReader::SkipChildElements).toInt();
            else if( name == QLatin1String("track") )
                m_track = reader->readElementText(QXmlStreamReader::SkipChildElements).toInt();
            else if( name == QLatin1String("bpm") )
                m_bpm = reader->readElementText(QXmlStreamReader::SkipChildElements).toFloat();
            else if( name == QLatin1String("bitrate") )
                m_bitrate = reader->readElementText(QXmlStreamReader::SkipChildElements).toInt();
            else if( name == QLatin1String("length") )
                m_length = reader->readElementText(QXmlStreamReader::SkipChildElements).toLong();
            else if( name == QLatin1String("samplerate") )
                m_samplerate = reader->readElementText(QXmlStreamReader::SkipChildElements).toInt();
            else if( name == QLatin1String("filesize") )
                m_filesize = reader->readElementText(QXmlStreamReader::SkipChildElements).toLong();
            else if( name == QLatin1String("mtime") )
                m_modified = QDateTime::fromTime_t(reader->readElementText(QXmlStreamReader::SkipChildElements).toLong());

            else if( name == QLatin1String("trackGain") )
                m_trackGain = reader->readElementText(QXmlStreamReader::SkipChildElements).toFloat();
            else if( name == QLatin1String("trackPeakGain") )
                m_trackPeakGain = reader->readElementText(QXmlStreamReader::SkipChildElements).toFloat();
            else if( name == QLatin1String("albumGain") )
                m_albumGain = reader->readElementText(QXmlStreamReader::SkipChildElements).toFloat();
            else if( name == QLatin1String("albumPeakGain") )
                m_albumPeakGain = reader->readElementText(QXmlStreamReader::SkipChildElements).toFloat();

            else if( name == QLatin1String("composer") )
                m_composer = reader->readElementText(QXmlStreamReader::SkipChildElements);

            else if( name == QLatin1String("rating") )
                m_rating = reader->readElementText(QXmlStreamReader::SkipChildElements).toFloat();
            else if( name == QLatin1String("score") )
                m_score = reader->readElementText(QXmlStreamReader::SkipChildElements).toFloat();
            else if( name == QLatin1String("playcount") )
                m_playcount = reader->readElementText(QXmlStreamReader::SkipChildElements).toInt();

            else
            {
                qDebug() << "Unexpected xml start element"<<name<<"in input";
                reader->skipCurrentElement();
            }
        }

        else if( reader->isEndElement() )
        {
            break;
        }
    }
}

/** Removes all characters not allowed by xml 1.0 specification.
    We need this because the Qt 4.6 xml scanner is behaving very badly
    when encountering such characters in the input.
    see http://en.wikipedia.org/wiki/Valid_Characters_in_XML
    */
QString
CollectionScanner::Track::escape( QString str ) const
{
    for( int i = 0; i < str.length(); ++i )
    {
        ushort c = str.at(i).unicode();
        if( (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D) ||
            (c > 0xD7FF && c < 0xE000) ||
            (c > 0xFFFD) )
            str[i] = '?';
    }
    return str;
}

void
CollectionScanner::Track::write( QXmlStreamWriter *writer,
                                 const QString &tag, const QString &str ) const
{
    if( !str.isEmpty() )
        writer->writeTextElement( tag, escape(str) );
}


void
CollectionScanner::Track::toXml( QXmlStreamWriter *writer ) const
{
    if( !m_valid )
        return;

    write( writer, QLatin1String("uniqueid"), m_uniqueid );
    write( writer, QLatin1String("path"), m_path );
    write( writer, QLatin1String("rpath"), m_rpath );

    write(writer, QLatin1String("filetype"), QString::number( (int)m_filetype ) );

    write( writer, QLatin1String("title"), m_title);
    write( writer, QLatin1String("artist"), m_artist);
    write( writer, QLatin1String("albumArtist"), m_albumArtist);
    write( writer, QLatin1String("album"), m_album);
    if( m_compilation )
        writer->writeEmptyElement( QLatin1String("compilation") );
    if( m_noCompilation )
        writer->writeEmptyElement( QLatin1String("noCompilation") );
    if( m_hasCover )
        writer->writeEmptyElement( QLatin1String("hasCover") );
    write( writer, QLatin1String("comment"), m_comment);
    write( writer, QLatin1String("genre"), m_genre);
    if( m_year != -1 )
        write(writer, QLatin1String("year"), QString::number( m_year ) );
    if( m_disc != -1 )
        write(writer, QLatin1String("disc"), QString::number( m_disc ) );
    if( m_track != -1 )
        write(writer, QLatin1String("track"), QString::number( m_track ) );
    if( m_bpm != -1 )
        write(writer, QLatin1String("bpm"), QString::number( m_bpm ) );
    if( m_bitrate != -1 )
        write(writer, QLatin1String("bitrate"), QString::number( m_bitrate ) );
    if( m_length != -1 )
        write(writer, QLatin1String("length"), QString::number( m_length ) );
    if( m_samplerate != -1 )
        write(writer, QLatin1String("samplerate"), QString::number( m_samplerate ) );
    if( m_filesize != -1 )
        write(writer, QLatin1String("filesize"), QString::number( m_filesize ) );
    if( m_modified.isValid() )
        write(writer, QLatin1String("mtime"), QString::number( m_modified.toTime_t() ) );

    if( m_trackGain != 0 )
        write(writer, QLatin1String("trackGain"), QString::number( m_trackGain ) );
    if( m_trackPeakGain != 0 )
        write(writer, QLatin1String("trackPeakGain"), QString::number( m_trackPeakGain ) );
    if( m_albumGain != 0 )
        write(writer, QLatin1String("albumGain"), QString::number( m_albumGain ) );
    if( m_albumPeakGain != 0 )
        write(writer, QLatin1String("albumPeakGain"), QString::number( m_albumPeakGain ) );

    write( writer, QLatin1String("composer"), m_composer);

    if( m_rating != -1 )
        write(writer, QLatin1String("rating"), QString::number( m_rating ) );
    if( m_score != -1 )
        write(writer, QLatin1String("score"), QString::number( m_score ) );
    if( m_playcount != -1 )
        write(writer, QLatin1String("playcount"), QString::number( m_playcount ) );

}

bool
CollectionScanner::Track::isValid() const
{
    return m_valid;
}

CollectionScanner::Directory*
CollectionScanner::Track::directory() const
{
    return m_directory;
}

QString
CollectionScanner::Track::uniqueid() const
{
    return m_uniqueid;
}

QString
CollectionScanner::Track::path() const
{
    return m_path;
}

QString
CollectionScanner::Track::rpath() const
{
    return m_rpath;
}

Amarok::FileType
CollectionScanner::Track::filetype() const
{
    return m_filetype;
}

QString
CollectionScanner::Track::title() const
{
    return m_title;
}

QString
CollectionScanner::Track::artist() const
{
    return m_artist;
}

QString
CollectionScanner::Track::albumArtist() const
{
    return m_albumArtist;
}

QString
CollectionScanner::Track::album() const
{
    return m_album;
}

bool
CollectionScanner::Track::isCompilation() const
{
    return m_compilation;
}

bool
CollectionScanner::Track::isNoCompilation() const
{
    return m_noCompilation;
}


bool
CollectionScanner::Track::hasCover() const
{
    return m_hasCover;
}

QString
CollectionScanner::Track::comment() const
{
    return m_comment;
}

QString
CollectionScanner::Track::genre() const
{
    return m_genre;
}

int
CollectionScanner::Track::year() const
{
    return m_year;
}

int
CollectionScanner::Track::disc() const
{
    return m_disc;
}

int
CollectionScanner::Track::track() const
{
    return m_track;
}

int
CollectionScanner::Track::bpm() const
{
    return m_bpm;
}

int
CollectionScanner::Track::bitrate() const
{
    return m_bitrate;
}

qint64
CollectionScanner::Track::length() const
{
    return m_length;
}

int
CollectionScanner::Track::samplerate() const
{
    return m_samplerate;
}

qint64
CollectionScanner::Track::filesize() const
{
    return m_filesize;
}

QDateTime
CollectionScanner::Track::modified() const
{
    return m_modified;
}


QString
CollectionScanner::Track::composer() const
{
    return m_composer;
}


qreal
CollectionScanner::Track::replayGain( Meta::ReplayGainTag mode ) const
{
    switch( mode )
    {
    case Meta::ReplayGain_Track_Gain:
        return m_trackGain;
    case Meta::ReplayGain_Track_Peak:
        return m_trackPeakGain;
    case Meta::ReplayGain_Album_Gain:
        return m_albumGain;
    case Meta::ReplayGain_Album_Peak:
        return m_albumPeakGain;
    }
    return 0.0;
}

qreal
CollectionScanner::Track::rating() const
{
    return m_rating;
}

qreal
CollectionScanner::Track::score() const
{
    return m_score;
}

int
CollectionScanner::Track::playcount() const
{
    return m_playcount;
}

void
CollectionScanner::Track::setUseCharsetDetector( bool value )
{
    s_useCharsetDetector = value;
}


