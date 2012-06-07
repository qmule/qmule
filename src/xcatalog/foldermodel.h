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

#ifndef MODEL_CATEGORY_H
#define MODEL_CATEGORY_H

#include <QtCore/QAbstractItemModel>
#include <QtGui/QImage>

class XCatalog;
class XFolder;

class FolderModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderModel)

public:
    FolderModel( XCatalog *catalog, QObject * parent = 0 );

    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const { Q_UNUSED(parent); return 1; }
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;
    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    virtual bool canFetchMore( const QModelIndex & parent ) const;
    virtual void fetchMore( const QModelIndex & parent );
    virtual void revert();

    inline bool indexValid( const QModelIndex &index ) const {
         return (index.row() >= 0) && (index.column() >= 0) && (index.model() == this);
    }

    static XFolder* indexToFolder( const QModelIndex &index )
        { return static_cast<XFolder*>(index.internalPointer()); }

private slots:
    void onFoldersFetched( XFolder *parent );

private:
    XCatalog *m_catalog;
    QImage m_imageFolder;
};

#endif // MODEL_CATEGORY_H
