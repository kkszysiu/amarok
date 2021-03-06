/****************************************************************************************
 * Copyright (c) 2009-2010 Bart Cerneels <bart.cerneels@kde.org>                        *
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

#define DEBUG_PREFIX "PlaylistBrowserCategory"

#include "PlaylistBrowserCategory.h"

#include "core-impl/collections/support/CollectionManager.h"
#include "core/support/Debug.h"
#include "PaletteHandler.h"
#include "playlist/PlaylistModel.h"
#include "playlistmanager/PlaylistManager.h"
#include "PlaylistsInFoldersProxy.h"
#include "PlaylistsByProviderProxy.h"
#include "PlaylistTreeItemDelegate.h"
#include "PlaylistBrowserFilterProxy.h"
#include "SvgHandler.h"
#include "statusbar/StatusBar.h"
#include "PlaylistBrowserView.h"

#include <KAction>
#include <KActionMenu>
#include <KButtonGroup>
#include <KIcon>
#include <KLineEdit>
#include <KConfigGroup>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <KStandardDirs>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include <typeinfo>

#include "PlaylistBrowserModel.h"

using namespace PlaylistBrowserNS;

QString PlaylistBrowserCategory::s_mergeViewKey( "Merged View" );

PlaylistBrowserCategory::PlaylistBrowserCategory( int playlistCategory,
                                                  QString categoryName,
                                                  QString configGroup,
                                                  PlaylistBrowserModel *model,
                                                  QWidget *parent ) :
    BrowserCategory( categoryName, parent ),
    m_configGroup( configGroup ),
    m_playlistCategory( playlistCategory )
{
    setContentsMargins( 0, 0, 0, 0 );
    m_toolBar = new KToolBar( this, false, false );
    m_toolBar->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    //a QWidget with minimumExpanding makes the next button right aligned.
    QWidget *spacerWidget = new QWidget( this );
    spacerWidget->setSizePolicy( QSizePolicy::MinimumExpanding,
                                 QSizePolicy::MinimumExpanding );
    // add a separator so subclasses can add their actions before it
    m_separator = m_toolBar->addWidget( spacerWidget );

    m_toolBar->addSeparator();

    m_addFolderAction = new KAction( KIcon( "folder-new" ), i18n( "Add Folder" ), this  );
    m_addFolderAction->setPriority( QAction::LowPriority );
    m_toolBar->addAction( m_addFolderAction );
    connect( m_addFolderAction, SIGNAL( triggered( bool ) ), SLOT( createNewFolder() ) );

    m_providerMenu = new KActionMenu( KIcon( "checkbox" ), i18n( "Visible Sources"), this );
    m_providerMenu->setDelayed( false );
    m_providerMenu->setPriority( QAction::HighPriority );
    m_toolBar->addAction( m_providerMenu );

    KAction *toggleAction = new KAction( KIcon( "view-list-tree" ), i18n( "Merged View" ),
                                         m_toolBar );
    toggleAction->setCheckable( true );
    toggleAction->setChecked( Amarok::config( m_configGroup ).readEntry( s_mergeViewKey, false ) );
    toggleAction->setPriority( QAction::LowPriority );
    m_toolBar->addAction( toggleAction );
    connect( toggleAction, SIGNAL( triggered( bool ) ), SLOT( toggleView( bool ) ) );

    m_toolBar->addSeparator();

    m_byProviderProxy = new PlaylistsByProviderProxy( model, PlaylistBrowserModel::ProviderColumn );
    m_byFolderProxy = new PlaylistsInFoldersProxy( model );

    m_filterProxy = new PlaylistBrowserFilterProxy( this );
    //no need to setModel on filterProxy since it will be done in toggleView anyway.
    m_filterProxy->setDynamicSortFilter( true );
    m_filterProxy->setFilterKeyColumn( PlaylistBrowserModel::ProviderColumn );

    m_playlistView = new PlaylistBrowserView( m_filterProxy, this );
    m_defaultItemDelegate = m_playlistView->itemDelegate();
    m_byProviderDelegate = new PlaylistTreeItemDelegate( m_playlistView );

    toggleView( toggleAction->isChecked() );

    m_playlistView->setFrameShape( QFrame::NoFrame );
    m_playlistView->setContentsMargins( 0, 0, 0, 0 );
    m_playlistView->setAlternatingRowColors( true );
    m_playlistView->header()->hide();
    //hide all columns except the first.
    for( int i = 1; i < m_playlistView->model()->columnCount(); i++ )
      m_playlistView->hideColumn( i );

    m_playlistView->setDragEnabled( true );
    m_playlistView->setAcceptDrops( true );
    m_playlistView->setDropIndicatorShown( true );

    foreach( const Playlists::PlaylistProvider *provider,
             The::playlistManager()->providersForCategory( m_playlistCategory ) )
    {
        createProviderButton( provider );
    }

    connect( The::playlistManager(), SIGNAL(providerAdded( Playlists::PlaylistProvider*, int )),
             SLOT(slotProviderAdded( Playlists::PlaylistProvider*, int )) );
    connect( The::playlistManager(), SIGNAL(providerRemoved( Playlists::PlaylistProvider*, int )),
             SLOT(slotProviderRemoved( Playlists::PlaylistProvider*, int )) );

    connect( The::paletteHandler(), SIGNAL( newPalette( const QPalette & ) ),
             SLOT( newPalette( const QPalette & ) ) );
}

PlaylistBrowserCategory::~PlaylistBrowserCategory()
{
}

QString
PlaylistBrowserCategory::filter() const
{
    return QUrl::toPercentEncoding( m_filterProxy->filterRegExp().pattern() );
}

void
PlaylistBrowserCategory::setFilter( const QString &filter )
{
    debug() << "Setting filter " << filter;
    m_filterProxy->setFilterRegExp( QRegExp( QUrl::fromPercentEncoding( filter.toUtf8() ) ) );
    //disable all other provider-buttons
    foreach( QAction * const providerAction, m_providerActions.values() )
    {
        const Playlists::PlaylistProvider *provider =
                providerAction->data().value<const Playlists::PlaylistProvider *>();
        if( provider )
            providerAction->setChecked(
                    m_filterProxy->filterRegExp().exactMatch( provider->prettyName() ) );
    }
}

QTreeView *
PlaylistBrowserCategory::playlistView()
{
    return m_playlistView;
}

void
PlaylistBrowserCategory::toggleView( bool merged )
{
    if( merged )
    {
        m_filterProxy->setSourceModel( m_byFolderProxy );
        m_playlistView->setItemDelegate( m_defaultItemDelegate );
        m_playlistView->setRootIsDecorated( true );
        m_addFolderAction->setHelpText( m_addFolderAction->text() );
    }
    else
    {
        m_filterProxy->setSourceModel( m_byProviderProxy );
        m_playlistView->setItemDelegate( m_byProviderDelegate );
        m_playlistView->setRootIsDecorated( false );
        m_addFolderAction->setHelpText( i18n( "Folders are only shown in <b>merged view</b>." ) );
    }

    //folders don't make sense in per-provider view
    m_addFolderAction->setEnabled( merged );

    Amarok::config( m_configGroup ).writeEntry( s_mergeViewKey, merged );
}

void
PlaylistBrowserCategory::slotProviderAdded( Playlists::PlaylistProvider *provider, int category )
{
    if( category != m_playlistCategory )
        return; //ignore

    if( !m_providerActions.keys().contains( provider ) )
        createProviderButton( provider );

    //Hide empty providers when added
    if( provider->playlistCount() != 0 )
        toggleView( false );

    slotToggleProviderButton();
}

void
PlaylistBrowserCategory::slotProviderRemoved( Playlists::PlaylistProvider *provider, int category )
{
    Q_UNUSED( category )

    if( m_providerActions.keys().contains( provider ) )
    {
        QAction *providerToggle = m_providerActions.take( provider );
        m_providerMenu->removeAction( providerToggle );
    }
}

void
PlaylistBrowserCategory::createProviderButton( const Playlists::PlaylistProvider *provider )
{
    QAction *providerToggle = new QAction( provider->icon(), provider->prettyName(), this );
    providerToggle->setCheckable( true );
    providerToggle->setChecked( true );
    providerToggle->setData( QVariant::fromValue( provider ) );
    connect( providerToggle, SIGNAL(toggled(bool)), SLOT(slotToggleProviderButton()) );
    m_providerMenu->addAction( providerToggle );

    //if there is only one provider the button needs to be disabled.
    //When a second is added we can enable the first.
    if( m_providerActions.count() == 0 )
        providerToggle->setEnabled( false );
    else if( m_providerActions.count() == 1 )
        m_providerActions.values().first()->setEnabled( true );

    //Hide empty providers when added
    if( provider->playlistCount() == 0 )
    {
        providerToggle->setChecked( false );
        //ensure the provider is actually filtered out.
        slotToggleProviderButton();
    }

    m_providerActions.insert( provider, providerToggle );
}

void
PlaylistBrowserCategory::slotToggleProviderButton()
{
    QString filter;
    QActionList checkedActions;
    foreach( const Playlists::PlaylistProvider *p, m_providerActions.keys() )
    {
        QAction *action = m_providerActions.value( p );
        if( action->isChecked() )
        {
            QString escapedName = QRegExp::escape( p->prettyName() ).replace( " ", "\\ " );
            filter += QString( filter.isEmpty() ? "%1" : "|%1" ).arg( escapedName );
            checkedActions << action;
            action->setEnabled( true );
        }
    }
    //if all are enabled the filter can be completely disabled.
    if( checkedActions.count() == m_providerActions.count() )
        filter = QString();

    m_filterProxy->setFilterRegExp( filter );

    //don't allow the last visible provider to be hidden
    if( checkedActions.count() == 1 )
        checkedActions.first()->setEnabled( false );
}

void
PlaylistBrowserCategory::createNewFolder()
{
    QString name = i18nc( "default name for new folder", "New Folder" );
    const QModelIndex &rootIndex = m_byFolderProxy->index(0,0);
    QModelIndexList folderIndices = m_byFolderProxy->match( rootIndex, Qt::DisplayRole, name, -1 );
    QString groupName = name;
    if( !folderIndices.isEmpty() )
    {
        QStringList names;
        foreach( const QModelIndex &folder, folderIndices )
            names << folder.data( Qt::DisplayRole ).toString();
        names.sort();
        QRegExp regex( "\\((\\d+)\\)" );
        int folderCount( 0 );
        int matchIndex = regex.indexIn( names.last() );
        if( matchIndex != -1 )
            folderCount = regex.cap( 1 ).toInt();
        groupName += QString( " (%1)" ).arg( folderCount + 1 );
    }
    QModelIndex idx = m_filterProxy->mapFromSource( m_byFolderProxy->createNewFolder( groupName ) );
    m_playlistView->setCurrentIndex( idx );
    m_playlistView->edit( idx );
}

void
PlaylistBrowserCategory::newPalette( const QPalette &palette )
{
    Q_UNUSED( palette )

    The::paletteHandler()->updateItemView( m_playlistView );
}

#include "PlaylistBrowserCategory.moc"
