/****************************************************************************************
 * Copyright (c) 2004-2008 Mark Kretschmann <kretschmann@kde.org>                       *
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

#define DEBUG_PREFIX "ConfigDialog"

#include "ConfigDialog.h"

#include "amarokconfig.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"

#include "CollectionConfig.h"
#include "DatabaseConfig.h"
#include "GeneralConfig.h"
//#include "MediadeviceConfig.h"
#include "NotificationsConfig.h"
#include "PlaybackConfig.h"
#include "PluginsConfig.h"
#include "ScriptsConfig.h"

#include <KLocalizedString>

QString Amarok2ConfigDialog::s_currentPage = "GeneralConfig";

//////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC
//////////////////////////////////////////////////////////////////////////////////////////

Amarok2ConfigDialog::Amarok2ConfigDialog( QWidget *parent, const char* name, KConfigSkeleton *config )
   : KConfigDialog( parent, name, config )
{
    DEBUG_BLOCK
    setAttribute( Qt::WA_DeleteOnClose );

    ConfigDialogBase* general     = new GeneralConfig( this );
    ConfigDialogBase* collection  = new CollectionConfig( this );
    ConfigDialogBase* plugins     = new PluginsConfig( this );
    ConfigDialogBase* playback    = new PlaybackConfig( this );
    ConfigDialogBase* notify      = new NotificationsConfig( this );
    ConfigDialogBase* database    = new DatabaseConfig( this );
    ConfigDialogBase* scripts     = new ScriptsConfig( this );

    //ConfigDialogBase* mediadevice = new MediadeviceConfig( this );

    addPage( general,     i18nc( "Miscellaneous settings", "General" ), "preferences-other-amarok", i18n( "Configure General Options" ) );
    addPage( collection,  i18n( "Collection" ), "collection-amarok", i18n( "Configure Collection" ) );
    addPage( playback,    i18n( "Playback" ), "preferences-media-playback-amarok", i18n( "Configure Playback" ) );
    addPage( notify,      i18n( "Notifications" ), "preferences-indicator-amarok", i18n( "Configure Notifications" ) );
    addPage( database,    i18n( "Database" ), "server-database", i18n( "Configure Database" ) );
    addPage( plugins,     i18n( "Plugins" ), "preferences-plugin", i18n( "Configure Plugins" ) );
    addPage( scripts,     i18n( "Scripts" ), "preferences-plugin-script", i18n( "Configure Scripts" ) );
    //addPage( mediadevice, i18n( "Media Devices" ), "preferences-multimedia-player-amarok", i18n( "Configure Portable Player Support" ) );

    setButtons( Help | Ok | Apply | Cancel );
    restoreDialogSize( Amarok::config( "ConfigDialog" ) );
}

Amarok2ConfigDialog::~Amarok2ConfigDialog()
{
    DEBUG_BLOCK

    KPageWidgetItem* pageItem = currentPage();

    foreach( ConfigDialogBase *configPage, m_pageList )
    {
        if( m_pageMap[configPage] == pageItem )
        {
            s_currentPage = configPage->metaObject()->className();
            break;
        }
    }

    KConfigGroup config = Amarok::config( "ConfigDialog" );
    saveDialogSize( config );
    AmarokConfig::self()->writeConfig();
}

void Amarok2ConfigDialog::updateButtons() //SLOT
{
    DEBUG_BLOCK

    enableButtonApply( hasChanged() );
}

/** Reimplemented from KConfigDialog */
void Amarok2ConfigDialog::addPage( ConfigDialogBase *page, const QString &itemName, const QString &pixmapName, const QString &header, bool manage )
{
    connect( page, SIGNAL( settingsChanged( const QString& ) ), this, SIGNAL( settingsChanged( const QString& ) ) );

    // Add the widget pointer to our list, for later reference
    m_pageList << page;
    KPageWidgetItem *pageWidget = KConfigDialog::addPage( page, itemName, pixmapName, header, manage );
    m_pageMap.insert( page, pageWidget );
}

void Amarok2ConfigDialog::show( QString page )
{
    if( page.isNull() )
    {
        page = s_currentPage;
    }

    foreach( ConfigDialogBase *configPage, m_pageList )
    {
        if( configPage->metaObject()->className() == page )
        {
            KPageWidgetItem *pageItem = m_pageMap[configPage];
            KConfigDialog::setCurrentPage( pageItem );
            break;
        }
    }

    KConfigDialog::show();
    raise();
    activateWindow();
}

//////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED SLOTS
//////////////////////////////////////////////////////////////////////////////////////////

/**
 * Update the settings from the dialog.
 * Example use: User clicks Ok or Apply button in a configure dialog.
 * REIMPLEMENTED
 */
void Amarok2ConfigDialog::updateSettings()
{
    foreach( ConfigDialogBase* page, m_pageList )
        page->updateSettings();
}

/**
 * Update the configuration-widgets in the dialog based on Amarok's current settings.
 * Example use: Initialisation of dialog.
 * Example use: User clicks Reset button in a configure dialog.
 * REIMPLEMENTED
 */
void Amarok2ConfigDialog::updateWidgets()
{
    foreach( ConfigDialogBase* page, m_pageList )
        page->updateWidgets();
}

/**
 * Update the configuration-widgets in the dialog based on the default values for Amarok's settings.
 * Example use: User clicks Defaults button in a configure dialog.
 * REIMPLEMENTED
 */
void Amarok2ConfigDialog::updateWidgetsDefault()
{
    foreach( ConfigDialogBase* page, m_pageList )
        page->updateWidgetsDefault();
}


//////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED
//////////////////////////////////////////////////////////////////////////////////////////

/**
 * @return true if any configuration items we are managing changed from Amarok's stored settings
 * We manage the engine combo box and some of the OSD settings
 * REIMPLEMENTED
 */
bool Amarok2ConfigDialog::hasChanged()
{
    DEBUG_BLOCK

    bool changed = false;

    foreach( ConfigDialogBase* page, m_pageList )
        if( page->hasChanged() ) {
            changed = true;
            debug() << "Changed: " << page->metaObject()->className();
        }

    return changed;
}

/** REIMPLEMENTED */
bool Amarok2ConfigDialog::isDefault()
{
    bool def = false;

    foreach( ConfigDialogBase* page, m_pageList )
        if( page->hasChanged() )
            def = true;

    return def;
}


//////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE SLOTS
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////////////////////


#include "ConfigDialog.moc"
