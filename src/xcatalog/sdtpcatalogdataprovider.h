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

#ifndef SDTPCATALOGDATAPROVIDER_H
#define SDTPCATALOGDATAPROVIDER_H

#include <QtCore/QSet>
#include <QtCore/QHash>
#include <QtCore/QTime>
#include "abstractcatalogdataprovider.h"

class QNetworkAccessManager;
class QNetworkReply;

class SdtpCatalogDataProvider : public AbstractCatalogDataProvider
{
    Q_OBJECT
    Q_DISABLE_COPY(SdtpCatalogDataProvider)

public:
    explicit SdtpCatalogDataProvider( QObject *parent = 0 );
    virtual ~SdtpCatalogDataProvider();

virtual void moveToThread2( QThread * targetThread );

public slots:
    virtual void fetch( int type, void *param );
    virtual void search( QString query, XFolderList selectedFolders );

private slots:
    void parseNetworkReply( QNetworkReply *reply );

private:
    QNetworkAccessManager *m_network;
    QHash< QNetworkReply*, QPair<void*,RequestType> > m_netRequests;

    void createRequest( QUrl &url, RequestType type, void *param );
    bool parseFoldersRecursiveData( XFolder* parent, QIODevice& stream, XFolderDict& folderDict, XFolderList& list );
    bool parseFoldersData( XFolder* parent, QIODevice& stream, XFolderList& list );
    bool parseFilesData( XFolder* parent, QIODevice& stream, XFileList& list );
    bool parseFileDetailsData( XFile* file, QIODevice& stream );
    bool parseSearchData( QIODevice& stream, XFileList& list);

#ifdef QT_DEBUG
    void debug_addRequestTime( QNetworkReply* reply );
    QHash<QNetworkReply*,QTime> m_debug_requestStart;
#endif
};

#endif // SDTPCATALOGDATAPROVIDER_H
