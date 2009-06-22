/***************************************************************************
 *   Plasma applet for showing videoclip in the context view.              *
 *                                                                         *
 *   Copyright (c) 2009 Simon Esneault <simon.esneault@gmail.com>          *
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

#include "VideoclipApplet.h" 
#include "VideoItemButton.h"

// Amarok
#include "Amarok.h"
#include "Debug.h"
#include "EngineController.h"
#include "meta/stream/Stream.h"
#include "collection/CollectionManager.h"
#include "context/ContextView.h"
#include "context/Svg.h"
#include "playlist/PlaylistController.h"
#include "SvgHandler.h"
#include "widgets/kratingpainter.h"
#include "widgets/kratingwidget.h"

// KDE
#include <KAction>
#include <KColorScheme>
#include <KConfigDialog>
#include <KMenu>
#include <KStandardDirs>
#include <KVBox>
#include <Plasma/Theme>
#include <Plasma/BusyWidget>
#include <Plasma/IconWidget>
#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/VideoWidget>

// Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QGraphicsWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QToolButton>
#include <QScrollArea>

#define DEBUG_PREFIX "VideoclipApplet"

Q_DECLARE_METATYPE ( VideoInfo *)
K_EXPORT_AMAROK_APPLET( videoclip, VideoclipApplet )


VideoclipApplet::VideoclipApplet( QObject* parent, const QVariantList& args )
        : Context::Applet( parent, args )
        , EngineObserver( The::engineController() )
        , m_settingsIcon( 0 )
        , m_youtubeHQ( false )
{
    DEBUG_BLOCK
    setHasConfigurationInterface( false );
    
    if( The::engineController() )
        m_mediaObject = const_cast<Phonon::MediaObject*>( The::engineController()->phononMediaObject() );
}


void 
VideoclipApplet::init()
{
    setBackgroundHints( Plasma::Applet::NoBackground );

    // HACK
    m_height = 300;
    
    // TODO inherit a phonon VideoWidget to unable mouse interaction (double click full screen etc ...)
    m_videoWidget = new Phonon::VideoWidget();
    m_videoWidget->setParent( Context::ContextView::self()->viewport(), Qt::SubWindow | Qt::FramelessWindowHint );
    m_videoWidget->hide();

    m_path = Phonon::createPath( m_mediaObject, m_videoWidget );
    if ( !m_path.isValid() )
        warning() << "Phonon path is invalid.";

    // Load pixmap
    m_pixYoutube = new QPixmap( KStandardDirs::locate( "data", "amarok/images/amarok-videoclip-youtube.png" ) );
    m_pixDailymotion = new QPixmap( KStandardDirs::locate( "data", "amarok/images/amarok-videoclip-dailymotion.png" ) );
    m_pixVimeo = new QPixmap( KStandardDirs::locate( "data", "amarok/images/amarok-videoclip-vimeo.png" ) );

    QAction* langAction = new QAction( i18n( "Settings" ), this );
    langAction->setIcon( KIcon( "preferences-system" ) );
    langAction->setVisible( true );
    langAction->setEnabled( true );
    m_settingsIcon = addAction( langAction );
    connect( m_settingsIcon, SIGNAL( activated() ), this, SLOT( showConfigurationInterface() ) );

    
    // Create label
    QFont labelFont;
    labelFont.setPointSize( labelFont.pointSize() + 2 );
    m_headerText = new QGraphicsSimpleTextItem( this );
    m_headerText->setBrush( Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor ) );
    m_headerText->setFont( labelFont );
    m_headerText->setText( i18n( "Video Clip" ) );

    // Create layout
    m_layout = new QHBoxLayout();
    m_layout->setSizeConstraint( QLayout::SetMinAndMaxSize );
    m_layout->setContentsMargins( 5, 5, 5, 5 );
    m_layout->setSpacing( 2 );

    // create a widget
    QWidget *window = new QWidget;
    window->setAttribute( Qt::WA_NoSystemBackground );
    window->setLayout( m_layout );

    // create a scroll Area
    QScrollArea *m_scroll = new QScrollArea();
    m_scroll->setMaximumHeight( m_height - m_headerText->boundingRect().height() - 4*standardPadding() );
    m_scroll->setWidget( window );
    m_scroll->setAttribute( Qt::WA_NoSystemBackground );
    m_scroll->viewport()->setAttribute( Qt::WA_NoSystemBackground );

    m_widget = new QGraphicsProxyWidget( this );
    m_widget->setWidget( m_scroll );

    constraintsEvent();

    connectSource( "videoclip" );
    connect( dataEngine( "amarok-videoclip" ), SIGNAL( sourceAdded( const QString & ) ),
             this, SLOT( connectSource( const QString & ) ) );

    engineNewTrackPlaying();// kickstart
}

VideoclipApplet::~VideoclipApplet()
{
    DEBUG_BLOCK
    delete m_videoWidget;
}

void 
VideoclipApplet::engineNewTrackPlaying()
{
    
    DEBUG_BLOCK
    if( The::engineController() && !m_mediaObject )
        m_mediaObject = const_cast<Phonon::MediaObject*>( The::engineController()->phononMediaObject() );
    if ( m_videoWidget && m_mediaObject && m_mediaObject->hasVideo() )
    {
        debug() << " VideoclipApplet | Show VideoWidget";
        m_path.reconnect( m_mediaObject, m_videoWidget );
        if ( !m_path.isValid() )
            warning() << "Phonon path is invalid.";
        m_videoWidget->show();
    }
    else if( m_videoWidget )
    {
        m_videoWidget->hide();
        m_path.reconnect( m_mediaObject, m_videoWidget );
        if ( !m_path.isValid() )
            warning() << "Phonon path is invalid.";        
    }
}

void 
VideoclipApplet::enginePlaybackEnded( int finalPosition, int trackLength, PlaybackEndedReason reason )
{
    Q_UNUSED( finalPosition )
    Q_UNUSED( trackLength )
    Q_UNUSED( reason )
    
    if( m_videoWidget )
	{
		m_widget->show();
        m_videoWidget->hide();
	}
}

void 
VideoclipApplet::constraintsEvent( Plasma::Constraints constraints )
{
    Q_UNUSED( constraints );
    prepareGeometryChange();

    m_headerText->setPos( size().width() / 2 - m_headerText->boundingRect().width() / 2, standardPadding() + 3 );
    m_widget->setPos( standardPadding(), m_headerText->pos().y() + m_headerText->boundingRect().height() + standardPadding() );
    m_widget->resize( size().width() - 2 * standardPadding(), size().height() - m_headerText->boundingRect().height() - 2*standardPadding() );
    m_videoWidget->setGeometry( QRect(
        pos().toPoint()+QPoint( standardPadding(), m_headerText->boundingRect().height() + 2.5 * standardPadding() ),
        size().toSize()-QSize( 2 * standardPadding(),  m_headerText->boundingRect().height() + 3.5 * standardPadding() ) ) );

    m_settingsIcon->setPos( size().width() - m_settingsIcon->size().width() - standardPadding(), standardPadding() );
}

void 
VideoclipApplet::paintInterface( QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect )
{
    Q_UNUSED( p );
    Q_UNUSED( option );
    Q_UNUSED( contentsRect );
    p->setRenderHint( QPainter::Antialiasing );
    // tint the whole applet
    addGradientToAppletBackground( p );
    // draw rounded rect around title
    drawRoundedRectAroundText( p, m_headerText );
 }

QSizeF 
VideoclipApplet::sizeHint( Qt::SizeHint which, const QSizeF & constraint ) const
{
    // hardcoding for now
    return QSizeF( QGraphicsWidget::sizeHint( which, constraint ).width(), m_height );
}

void 
VideoclipApplet::connectSource( const QString &source )
{
    if ( source == "videoclip" )
        dataEngine( "amarok-videoclip" )->connectSource( "videoclip", this );
}

void 
VideoclipApplet::dataUpdated( const QString& name, const Plasma::DataEngine::Data& data ) // SLOT
{
    DEBUG_BLOCK
    Q_UNUSED( name )
    
    // HACK sometimes it takes longer for amarok to realize that a stream has video,
    // so when engineNewTrackPlaying is called it doesn't know about it yet. however
    // by the time this gets called, more has been downloaded and phonon figures it outs
    engineNewTrackPlaying();

    if ( data.empty() )
        return;
    
    if ( !m_videoWidget->isVisible() )
    {
        int width = 130;
        // Properly delete previsouly allocated item
        while ( !m_layoutWidgetList.empty() )
        {
            m_layoutWidgetList.front()->hide();
            m_layout->removeWidget( m_layoutWidgetList.front() );
			delete m_layoutWidgetList.front();
            m_layoutWidgetList.pop_front();
        }
        
        // if we get a message, show it
        if ( data.contains( "message" ) && data["message"].toString().contains("Fetching"))
			setBusy( true );
		else if ( data.contains( "message" ) )
		{
			QLabel *mess = new QLabel( data["message"].toString() );
            mess->setAlignment(Qt::AlignTop);
            m_layout->addWidget( mess, Qt::AlignTop );
            m_layoutWidgetList.push_back( mess );		
			setBusy( false );
		}
        else if ( data.contains( "item:0" ) )
        {
			setBusy(false);
            for (int i=0; i< data.size(); i++ )
            {
                VideoInfo *item = data[ QString ("item:" )+QString().setNum(i) ].value<VideoInfo *>() ;
                if( item->url != "" ) // prevent some weird stuff ...
                {

                    VideoItemButton *vidButton = new VideoItemButton();
                    vidButton->setVideoInfo( item );
                    
                    connect ( vidButton, SIGNAL( appendRequested( VideoInfo * ) ), this, SLOT ( appendVideoClip( VideoInfo * ) ) );
                    connect ( vidButton, SIGNAL( queueRequested( VideoInfo* ) ), this, SLOT ( queueVideoClip( VideoInfo * ) ) );
                    connect ( vidButton, SIGNAL( appendPlayRequested( VideoInfo * ) ), this, SLOT ( appendPlayVideoClip( VideoInfo * ) ) );

                    // create link (and resize, no more than 3 lines long)
                    QString title( item->title );
                    if ( title.size() > 45 ) title.resize( 45 );
                    QLabel *link = new QLabel( QString( "<html><body><a href=\"" ) + item->url + QString( "\">" ) + title + QString( "</a>" ) );
                    link->setOpenExternalLinks( true );
                    link->setWordWrap( true );

                    QLabel *duration =  new QLabel( item->duration + QString( "<br>" ) + item->views + QString( " views" ) );

                    KRatingWidget* rating = new KRatingWidget;
                    rating->setRating(( int )( item->rating * 2. ) );
                    rating->setMaximumWidth(( int )(( width / 3 )*2 ) );
                    rating->setMinimumWidth(( int )(( width / 3 )*2 ) );

                    QLabel *webi = new QLabel;
                    if ( item->source == QString( "youtube" ) )
                        webi->setPixmap( *m_pixYoutube );
                    else if ( item->source == QString( "dailymotion" ) )
                        webi->setPixmap( *m_pixDailymotion );
                    else if ( item->source == QString( "vimeo" ) )
                        webi->setPixmap( *m_pixVimeo );


                    QGridLayout *grid = new QGridLayout();
                    grid->setHorizontalSpacing( 5 );
                    grid->setVerticalSpacing( 2 );
                    grid->setRowMinimumHeight( 1, 65 );
                    grid->setColumnStretch( 0, 0 );
                    grid->setColumnStretch( 1, 1 );
                    grid->addWidget( vidButton, 0, 0, 1, -1, Qt::AlignCenter );
                    grid->addWidget( link, 1, 0, 1, -1, Qt::AlignCenter | Qt::AlignTop );
                    grid->addWidget( webi, 2, 0, Qt::AlignCenter );
                    grid->addWidget( duration, 2, 1, Qt::AlignLeft );
                    grid->addWidget( rating, 3, 0, 1, -1, Qt::AlignCenter );

                    // Add The Widget
                    QWidget *widget = new QWidget();
                    widget->setLayout( grid );
                    widget->resize( width, m_height - m_headerText->boundingRect().height() - 2*standardPadding() );
                    widget->setMaximumWidth( width );
                    widget->setMinimumWidth( width );
                    widget->setMinimumHeight( m_height - ( m_headerText->boundingRect().height() + 10 * standardPadding() ) );
                    widget->setMaximumHeight( m_height - ( m_headerText->boundingRect().height() + 10 * standardPadding() ) );
                    m_layout->addWidget( widget, Qt::AlignLeft );
                    m_layoutWidgetList.push_back( widget );

                    if ( i < data.size() - 1 )
                    {
                        QFrame *line = new QFrame();
                        line->setFrameStyle( QFrame::VLine );
                        line->setAutoFillBackground( false );
                        line->setMaximumHeight( m_height - ( m_headerText->boundingRect().height() + 2 * standardPadding() ) );
                        m_layout->addWidget( line, Qt::AlignLeft );
                        m_layoutWidgetList.push_back( line );
                    }
                }
            }
        }
    }
    updateConstraints();
}

void 
VideoclipApplet::appendVideoClip( VideoInfo *info )
{
	DEBUG_BLOCK
    QAbstractButton *button = qobject_cast<QAbstractButton *>(QObject::sender() );
    if ( button )
    {
        QStringList lst = button->text().split(" | ");
    
        MetaStream::Track *tra = new MetaStream::Track(KUrl( info->videolink ) );
        tra->setTitle( info->title );
        tra->setAlbum( info->source );
        tra->setArtist( info->artist );
        tra->album()->setImage( *info->cover );
        Meta::TrackPtr track( tra );
        //append to the playlist the newly retrieved
        The::playlistController()->insertOptioned(track , Playlist::Append );
    }
}

void
VideoclipApplet::queueVideoClip( VideoInfo *info )
{
    DEBUG_BLOCK
    QAbstractButton *button = qobject_cast<QAbstractButton *>(QObject::sender() );
    if ( button )
    {
        QStringList lst = button->text().split(" | ");
        
        MetaStream::Track *tra = new MetaStream::Track(KUrl( info->videolink ) );
        tra->setTitle( info->title );
        tra->setAlbum( info->source );
        tra->setArtist( info->artist );
        tra->album()->setImage( *info->cover );
        Meta::TrackPtr track( tra );
        //append to the playlist the newly retrieved
        The::playlistController()->insertOptioned(track , Playlist::Queue );
    }
}

void
VideoclipApplet::appendPlayVideoClip( VideoInfo *info )
{
    DEBUG_BLOCK
    QAbstractButton *button = qobject_cast<QAbstractButton *>(QObject::sender() );
    if ( button )
    {
        QStringList lst = button->text().split(" | ");
        
        MetaStream::Track *tra = new MetaStream::Track(KUrl( info->videolink ) );
        tra->setTitle( info->title );
        tra->setAlbum( info->source );
        tra->setArtist( info->artist );
        tra->album()->setImage( *info->cover );
        Meta::TrackPtr track( tra );
        //append to the playlist the newly retrieved
        The::playlistController()->insertOptioned(track , Playlist::AppendAndPlayImmediately );
    }
}

void
VideoclipApplet::videoMenu( QPoint point )
{
    KMenu *men = new KMenu(m_videoWidget);
    if ( !m_videoWidget->isFullScreen() )
    {
        KAction *toggle = new KAction( KIcon( "view-fullscreen" ), i18n( "Enter &fullscreen" ), this );
        men->addAction( toggle );
        connect( toggle, SIGNAL( triggered(bool) ), m_videoWidget, SLOT( exitFullScreen() ) );
    }
    else
    {
        KAction *toggle = new KAction( KIcon( "edit-undo" ), i18n( "E&xit fullscreen" ), this );
        men->addAction( toggle );
        connect( toggle, SIGNAL( triggered(bool) ), m_videoWidget, SLOT( exitFullScreen() ) );
    }   
    men->exec( m_videoWidget->mapToGlobal( point ) );
}

Plasma::IconWidget *
VideoclipApplet::addAction( QAction *action )
{
    DEBUG_BLOCK
    if ( !action ) {
        debug() << "ERROR!!! PASSED INVALID ACTION";
        return 0;
    }
    
    Plasma::IconWidget *tool = new Plasma::IconWidget( this );
    tool->setAction( action );
    tool->setText( "" );
    tool->setToolTip( action->text() );
    tool->setDrawBackground( false );
    tool->setOrientation( Qt::Horizontal );
    QSizeF iconSize = tool->sizeFromIconSize( 16 );
    tool->setMinimumSize( iconSize );
    tool->setMaximumSize( iconSize );
    tool->resize( iconSize );
    tool->setZValue( zValue() + 1 );
    
    return tool;
}

void
VideoclipApplet::createConfigurationInterface( KConfigDialog *parent )
{
    KConfigGroup configuration = config();
    QWidget *settings = new QWidget;
    ui_Settings.setupUi( settings );
    
    // TODO bad, it's done manually ...
    if ( m_youtubeHQ == true )
        ui_Settings.checkBox->setChecked( true );
    
    parent->addPage( settings, i18n( "Video Clip Settings" ), "preferences-system");
    //connect( ui_Settings, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( switchToLang( QString ) ) );
}



#include "VideoclipApplet.moc"

