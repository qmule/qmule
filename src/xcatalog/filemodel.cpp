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

#include "config.h"
#include "filemodel.h"
#include "catalog.h"

FileModel::FileModel( XCatalog *dataSource, QObject *parent ) :
    QAbstractItemModel(parent),
    m_dataSource(dataSource),
    m_imageThumb(QLatin1String(":/images/puzzle.png"))
{
}

FileModel::~FileModel()
{
}

int FileModel::rowCount( const QModelIndex &parent ) const
{
    if ( parent != QModelIndex() || parent.column() > 0 )
        return 0;

    return m_dataSource->selectedFolder() ? m_dataSource->selectedFolder()->files().count() : 0;
}

QVariant FileModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() || index.model() != this )
        return QVariant();

    switch ( role )
    {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
        return QString("%1")
                .arg(indexToFile(index)->name());


    case Qt::DecorationRole:
    {
        XFile *file = indexToFile(index);
        if ( None == file->thumbFetchStatus() ) {
            m_dataSource->fetchThumbnail(file);
            return m_imageThumb;
        } else if ( !file->thumbnail().isNull() ) {
            return file->thumbnail();
        } else {
            return m_imageThumb;
        }
    }

    case XFileRole:
        return qVariantFromValue( indexToFile(index) );

    case DateRole:
        return indexToFile(index)->date();

    case PathRole:
        return m_dataSource->createFolderPath(indexToFile(index)->parent());

    default:
        return QVariant();
    }
}

QModelIndex FileModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

bool FileModel::hasChildren( const QModelIndex &parent ) const
{
    if( !parent.isValid() ) // root
        return true;
    else // files
        return false;
}

QModelIndex FileModel::index( int row, int column, const QModelIndex & parent ) const
{
    if ( row < 0 || row >= rowCount(parent) || column < 0 || column >= columnCount(parent) )
        return QModelIndex();

    return createIndex(row, column, (void*)m_dataSource->selectedFolder()->files().at(row));
}

int FileModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

void FileModel::revert()
{
    reset();
}
