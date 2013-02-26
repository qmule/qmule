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

#ifndef ABSTRACTCATALOGDATAPROVIDER_H
#define ABSTRACTCATALOGDATAPROVIDER_H

#include <QtCore/QObject>
#include <QtGui/QImage>

#include "folder.h"

class AbstractCatalogDataProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractCatalogDataProvider)

public:

    explicit AbstractCatalogDataProvider( QObject *parent = 0 );
    virtual ~AbstractCatalogDataProvider();

    virtual void moveToThread2( QThread* thread ) { moveToThread(thread); }

signals:
    void data( int type, void* param, void* data );
    void error( int type, void* param );

public slots:
    virtual void fetch( int type, void* param ) = 0;
    virtual void search( QString query, XFolderList selectedFolders ) = 0;
};

#endif // ABSTRACTCATALOGDATAPROVIDER_H
