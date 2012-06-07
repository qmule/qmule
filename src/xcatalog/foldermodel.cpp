//this file is part of xCatalog
//Copyright (C) 2011 xCatalog Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "foldermodel.h"
#include "catalog.h"

FolderModel::FolderModel( XCatalog *catalog, QObject * parent ) :
    QAbstractItemModel(parent),
    m_catalog(catalog),
    m_imageFolder(":/images/folder.png")
{
    connect(m_catalog, SIGNAL(foldersFetched(XFolder*)),
            this, SLOT(onFoldersFetched(XFolder*)) );
}

int FolderModel::rowCount( const QModelIndex &parent ) const
{
    if ( parent.column() > 0 )
        return 0;

    if ( !parent.isValid() )
        return m_catalog->rootFolder()->folders().count();

    return indexToFolder(parent)->folders().count();
}

QVariant FolderModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() || index.model() != this )
        return QVariant();

    switch ( role )
    {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return indexToFolder(index)->name();

    case Qt::DecorationRole:
        return m_imageFolder;

    }

    return QVariant();
}

QModelIndex FolderModel::parent( const QModelIndex & index ) const
{
    if ( !indexValid(index) )
        return QModelIndex();

    const XFolder *folder = indexToFolder(index);
    Q_ASSERT(folder != 0);
    const XFolder *parentFolder = (folder ? folder->parent() : NULL);
    if ( 0 == parentFolder || parentFolder == m_catalog->rootFolder() )
        return QModelIndex();

    // get the parent's row
    const XFolder *grandParentFolder = parentFolder->parent();
    Q_ASSERT(grandParentFolder->folders().contains((XFolder*)parentFolder));

    return createIndex(grandParentFolder->folders().indexOf((XFolder*)parentFolder), 0, (void*)parentFolder);
}

bool FolderModel::hasChildren( const QModelIndex &parent ) const
{
    if ( parent.column() > 0 )
        return false;

    const XFolder *folder = indexToFolder(parent);
    if ( NULL == folder ) {
        folder = m_catalog->rootFolder();
    }

    return folder->hasChildren();
}

QModelIndex FolderModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent) )
        return QModelIndex();

    const XFolder *parentFolder = indexValid(parent) ? indexToFolder(parent) :
                                  m_catalog->rootFolder();
    Q_ASSERT(parentFolder != 0);

    return createIndex(row, column, (void*)parentFolder->folders().at(row));
}

bool FolderModel::canFetchMore( const QModelIndex &parent ) const
{
    const XFolder *folder = parent.isValid() ? indexToFolder(parent) : m_catalog->rootFolder();

    if ( folder->hasChildren() && None == folder->foldersFetchStatus() )
        return true;
    else
        return false;
}

void FolderModel::fetchMore( const QModelIndex &parent )
{
    XFolder *folder;

    if( parent.isValid() )
        folder = indexToFolder(parent);
    else
        folder = m_catalog->rootFolder();

    m_catalog->fetchFolders(folder);
}

void FolderModel::onFoldersFetched( XFolder *parentFolder )
{
    QModelIndex parent;

    if ( m_catalog->rootFolder() == parentFolder ) {
        parent = QModelIndex();
    } else {
        int row = parentFolder->parent()->folders().indexOf((XFolder*)parentFolder);
        parent = createIndex(row, 0, (void*)parentFolder);
    }

    beginInsertRows(parent, 0, parentFolder->folders().count());
    endInsertRows();
}

void FolderModel::revert()
{
    reset();
}
