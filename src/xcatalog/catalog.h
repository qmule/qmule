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

#ifndef CATALOG_H
#define CATALOG_H

#include <QtCore/QObject>
#include <QtGui/QImage>

#include "folder.h"

class AbstractCatalogDataProvider;

class XCatalog: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(XCatalog)

public:
    XCatalog( QObject *parent = NULL );
    virtual ~XCatalog();

    XFolder* rootFolder() const { return m_rootFolder; }

    XFolder* selectedFolder() const { return m_selectedFolder; }
    void setSelectedFolder( XFolder *folder ) { m_selectedFolder = folder; }

    XFile* selectedFile() const { return m_selectedFile; }
    void setSelectedFile( XFile *folder ) { m_selectedFile = folder; }

    const XFolderDict folderDict() const { return m_folderDict; }

    QString createFolderPath( const XFolder *folder ) const;


signals:
    void allFoldersFetched( XFolder *parent );
    void foldersFetched( XFolder *parent );
    void filesFetched( XFolder *parent );
    void fileDetailsFetched( XFile *file );
    void thumbnailFetched( XFile *file );
    void searchComplete( XFolder *result );

    void queueFetch( int type, void* param);
    void queueSearch( QString query, XFolderList selectedFolders );
    void error( int type, void* param );

public slots:
    void reset();
    void fetchFoldersRecursive( XFolder *parent );
    void fetchFolders( XFolder *parent );
    void fetchFiles( XFolder *parent );
    void fetchFileDetails( XFile *file );
    void fetchThumbnail( XFile *file );
    void execSearch( QString query, XFolderList selectedFolders );

private slots:
    void onDataFetched( int type, void* param, void* data );
    void onError( int type, void* param );

private:
    QString cachedThumbnail( const XFile *file ) const;
    QImage createThumbnail( const QImage& image );

    AbstractCatalogDataProvider *m_dataProvider;
    QThread *m_dataProviderThread;
    QString m_imageCachePath;
    XFolder *m_rootFolder;
    XFolder *m_selectedFolder;
    XFile *m_selectedFile;
    XFolderDict m_folderDict;
};

#endif // CATALOG_H
