/***************************************************************************
   begin                : Tue Feb 4 2003
   copyright            : (C) 2003 Scott Wheeler <wheeler@kde.org>
                        : (C) 2004 Max Howell <max.howell@methylblue.com>
                        : (C) 2004 Mark Kretschmann <markey@web.de>
                        : (C) 2008 Seb Ruiz <ruiz@kde.org>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AMAROK_COLLECTIONSETUP_H
#define AMAROK_COLLECTIONSETUP_H

#include <KVBox>      //baseclass

#include <QCheckBox>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTreeWidgetItem>

#include "Debug.h"

namespace CollectionFolder { class Model; }

// Reimplement sizeHint to have directorylist not being too big for "low" (1024x768 is not exactly low) resolutions
class CollectionView : public QTreeView
{
    public:
        explicit CollectionView( QWidget * parent = 0 )
            : QTreeView( parent )
        { }

        QSize sizeHint() const
        {
            return QSize( 400, 150 );
        }
};

class CollectionSetup : public KVBox
{
    friend class CollectionFolder::Model;

    public:
        static CollectionSetup* instance() { return s_instance; }

        CollectionSetup( QWidget* );
        void writeConfig();
    
        QStringList dirs() const { return m_dirs; }
        bool recursive() const { return m_recursive && m_recursive->isChecked(); }
        bool monitor() const { return m_monitor && m_monitor->isChecked(); }

    private:
        static CollectionSetup* s_instance;

        CollectionView *m_view;
        QStringList m_dirs;
        QCheckBox *m_recursive;
        QCheckBox *m_monitor;
};


namespace CollectionFolder //just to keep it out of the global namespace
{
    class Model : public QFileSystemModel
    {
        public:
            Model();
        
            virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
            QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
            bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

            virtual int columnCount( const QModelIndex& ) const { return 1; }

        private:
            bool ancestorChecked( const QString &path ) const;
            bool recursive() const { return CollectionSetup::instance()->recursive(); } // simply for convenience
            QSet<QString> m_checked;
    };

} // end namespace CollectionFolder

#endif

