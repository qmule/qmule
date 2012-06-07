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

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtGui/QImageReader>
#include <QtCore/QUrl>
#include "3rdparty/sdtp/sdtp.h"

#include "sdtpcatalogdataprovider.h"
#include "config.h"

#define SDTP_PROTO  "http"
#define SDTP_SERVER "webemule.is74.ru"
#define SDTP_SCRIPT "/sdtp3.php"

#define SDTP_URL_GETALLCATS SDTP_PROTO "://" SDTP_SERVER SDTP_SCRIPT "?a=allfolders"
#define SDTP_URL_GETCATS    SDTP_PROTO "://" SDTP_SERVER SDTP_SCRIPT "?a=folders&id=%1"
#define SDTP_URL_GETFILES   SDTP_PROTO "://" SDTP_SERVER SDTP_SCRIPT "?a=files&id=%1"
#define SDTP_URL_GETDETAILS SDTP_PROTO "://" SDTP_SERVER SDTP_SCRIPT "?a=details&id=%1"
#define SDTP_URL_SEARCH     SDTP_PROTO "://" SDTP_SERVER SDTP_SCRIPT "?a=search&query=%1&ids=%2"

SdtpCatalogDataProvider::SdtpCatalogDataProvider(QObject *parent) :
        AbstractCatalogDataProvider(parent),
        m_network(new QNetworkAccessManager)
{
    connect(m_network, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(parseNetworkReply(QNetworkReply*)));
}

SdtpCatalogDataProvider::~SdtpCatalogDataProvider()
{
    delete m_network;
}

void SdtpCatalogDataProvider::moveToThread2( QThread *thread )
{
    qDebug("DataProvider moved to thread %p", thread);

    AbstractCatalogDataProvider::moveToThread(thread);
    m_network->moveToThread(thread);
}

void SdtpCatalogDataProvider::fetch(int type, void *param)
{
    switch(type) {
    case FoldersRecursive:
    {
        XFolder* parent = static_cast<XFolder*>(param);
        Q_CHECK_PTR(parent);

        QUrl url( QString(SDTP_URL_GETALLCATS) );
        createRequest(url, FoldersRecursive, parent);

        break;
    }

    case Folders:
    {
        XFolder* parent = static_cast<XFolder*>(param);
        Q_CHECK_PTR(parent);

        QUrl url( QString(SDTP_URL_GETCATS).arg(parent->id()) );
        createRequest(url, Folders, parent);

        break;
    }

    case Files:
    {
        XFolder* parent = static_cast<XFolder*>(param);
        Q_CHECK_PTR(parent);

        QUrl url( QString(SDTP_URL_GETFILES).arg(parent->id()) );
        createRequest(url, Files, parent);

        break;
    }

    case FileDetails:
    {
        XFile* file = static_cast<XFile*>(param);
        Q_CHECK_PTR(file);

        QUrl url( QString(SDTP_URL_GETDETAILS).arg(file->id()) );
        createRequest(url, FileDetails, file);

        break;
    }

    case Thumbnail:
    {
        XFile* file = static_cast<XFile*>(param);
        Q_CHECK_PTR(file);

        QUrl url( file->thumbLink() );
        createRequest(url, Thumbnail, file);

        break;
    }

    default:
        qDebug("Unknown request type '%d'", static_cast<quint32>(type));
    }

}

void SdtpCatalogDataProvider::createRequest( QUrl& url, RequestType type, void* param )
{
    // already requested?
    QHash<QNetworkReply*, QPair<void*, RequestType> >::const_iterator it = m_netRequests.begin();
    while( it != m_netRequests.end() ) {
        QPair<void*, RequestType> pair = it.value();
        if ( pair.first == param && pair.second == type)
            return;
        ++it;
    }

    QNetworkReply *reply = m_network->get(QNetworkRequest(url));
#ifdef QT_DEBUG
    debug_addRequestTime(reply);
#endif
    m_netRequests[reply] = qMakePair(param, type);
}

void SdtpCatalogDataProvider::search( QString query, XFolderList selectedFolders )
{
    QString idString;
    for ( XFolderList::const_iterator it = selectedFolders.constBegin();
          it != selectedFolders.constEnd(); ++it ) {
        idString += QString("%1,").arg((*it)->id());
    }
    idString.chop(1);

    QUrl url( QString(SDTP_URL_SEARCH).arg(query).arg(idString) );

    createRequest(url, Search, NULL);
}

void SdtpCatalogDataProvider::parseNetworkReply( QNetworkReply *reply )
{
    //
    //
    //
    if( !m_netRequests.contains(reply) ) {
        qDebug("Orphan request found with url = '%s'",
               reply->url().toString().toLatin1().constData());
        reply->deleteLater();
        return;
    }

#ifdef QT_DEBUG
    qDebug("Network request '%s' took %d ms",
           reply->url().toString().toLatin1().constData(),
           m_debug_requestStart[reply].elapsed() );
    m_debug_requestStart[reply].restart();

    if ( reply->error() != QNetworkReply::NoError ) {
        qDebug("QNetworkReply error! Code: %d", reply->error());
    }
#endif

    QPair<void*, RequestType> pair = m_netRequests[reply];
    void* param = pair.first;
    RequestType type = pair.second;

    //
    // HANDLE REDIRECTION
    //
    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if ( !redirectUrl.isEmpty() && redirectUrl != reply->url()) {
        // all non-local thumbnail redirects are disabled!
        QString redirUrl = redirectUrl.toString();
        if ( Thumbnail != type || !redirUrl.contains(QRegExp("is74.ru/", Qt::CaseInsensitive, QRegExp::Wildcard)) )
        {
            QNetworkReply* newReply = m_network->get(QNetworkRequest(redirectUrl));
#ifdef QT_DEBUG
            debug_addRequestTime(newReply);
#endif
            // replace reply ptr in hash
            m_netRequests[newReply] = m_netRequests[reply];
            m_netRequests.remove(reply);
            return;
        }
    }

    m_netRequests.remove(reply);

    switch ( type ) {
    case FoldersRecursive: {
        XFolder* parent = static_cast<XFolder*>(param);
        XFolderList *list = new XFolderList;
        XFolderDict *dict = new XFolderDict;
        bool ok = parseFoldersRecursiveData(parent, *reply, *dict, *list);
        if ( ok ) {
            emit data(FoldersRecursive, parent, list);
            emit data(FoldersRecursiveDictionary, parent, dict);
        } else {
            delete list;
            delete dict;
            emit error(FoldersRecursive, parent);
        }
        break;
    }

    case Folders: {
        XFolder* parent = static_cast<XFolder*>(param);
        XFolderList *list = new XFolderList;
        bool ok = parseFoldersData(parent, *reply, *list);
        if ( ok )
            emit data(Folders, parent, list);
        else {
            delete list;
            emit error(Folders, parent);
        }
        break;
    }

    case Files: {
        XFolder *parent = static_cast<XFolder*>(param);
        XFileList *list = new XFileList;
        bool ok = parseFilesData(parent, *reply, *list);
        if ( ok )
            emit data(Files, parent, list);
        else {
            delete list;
            emit error(Files, parent);
        }
        break;
    }

    case FileDetails: {
        XFile *file = static_cast<XFile*>(param);
        XFile *dummy = new XFile(0,0,QString());;
        bool ok = parseFileDetailsData(dummy, *reply);
        if ( ok )
            emit data(FileDetails, file, dummy);
        else {
            delete dummy;
            emit error(FileDetails, file);
        }
        break;
    }

    case Thumbnail: {
        XFile *file = static_cast<XFile*>(param);
        QImageReader imgReader(reply);
        QImage *image = new QImage();
        *image = imgReader.read();
        if ( !image->isNull() ) {
            emit data(Thumbnail, file, image);
        } else {
            delete image;
            emit error(Thumbnail, file);
        }

        break;
    }

    case Search: {
        XFileList *list = new XFileList;
        bool ok = parseSearchData(*reply, *list);
        if ( ok )
            emit data(Search, NULL, list);
        else {
            delete list;
            emit error(Search, NULL);
        }

        break;
    }

    case FoldersRecursiveDictionary:
    default:
    {
        qDebug("Unsupported RequetType reply '%d'", type);
    }

    }

#ifdef QT_DEBUG
    qDebug("Parsing took %d ms", m_debug_requestStart[reply].elapsed() );
    m_debug_requestStart.remove(reply);
#endif

    reply->deleteLater();
}

bool SdtpCatalogDataProvider::parseFoldersRecursiveData( XFolder* parent, QIODevice& stream,
        XFolderDict& folderDict, XFolderList& list )
{
    folderDict[parent->id()] = parent;

    while ( stream.bytesAvailable() > 0 ) {
        quint32 id, pid, children;
        QString name;

        try{
            id = SDTP::unpackUInt32(stream);
            pid = SDTP::unpackUInt32(stream);
            children = SDTP::unpackUInt32(stream);
            name = SDTP::unpackString(stream);
        } catch( std::exception *e ) {
            qDebug("Failed to parse 'allfolder' packet");
            for ( XFolderList::iterator it = list.begin(); it != list.end(); ++it )
                delete *it;
            folderDict.clear();
            delete e;
            return false;
        }

        parent = folderDict[pid];
        if ( NULL == parent ) {
            qDebug("Folder id=%u with pid=%u hasn't parent in dictionary: skipping", id, pid);
            continue;
        }

        XFolder* folder = new XFolder(id, name, children, parent);
        folder->setFoldersFetchStatus(Done);
        parent->addFolder(folder);

        if( 0 == pid ) // add to root list
            list.append(folder);
        folderDict[id] = folder;
    }

    return true;
}

bool SdtpCatalogDataProvider::parseFoldersData( XFolder* parent, QIODevice &stream, XFolderList& list )
{
    while ( stream.bytesAvailable() > 0 ) {
        quint32 id, children;
        QString name;

        try {
            id = SDTP::unpackUInt32(stream);
            children = SDTP::unpackUInt32(stream);
            name = SDTP::unpackString(stream);
        } catch ( std::exception *e ) {
            qDebug("Failed to parse 'folder' packet");
            delete e;
            for ( XFolderList::iterator it = list.begin(); it != list.end(); ++it )
                delete *it;
            return false;
        }

        XFolder* folder = new XFolder(id, name, children, parent);
        list.append(folder);
    }

    return true;
}

bool SdtpCatalogDataProvider::parseFilesData( XFolder* parent, QIODevice &stream, XFileList& list )
{
    while ( stream.bytesAvailable() > 0 ) {
        quint32 id, date;
        QString name, thumbLink;

        try {
            id = SDTP::unpackUInt32(stream);
            name = SDTP::unpackString(stream);
            thumbLink = SDTP::unpackString(stream);
            date = SDTP::unpackUInt32(stream);
        } catch( std::exception *e ) {
            qDebug("Failed to parse 'files' packet");
            delete e;
            for ( XFileList::iterator it = list.begin(); it != list.end(); ++it )
                delete *it;
            return false;
        }

        XFile *file = new XFile(id, parent, name);
        file->setThumbLink(thumbLink);
        file->setDate(date);

        list.append(file);
    }

    return true;
}

bool SdtpCatalogDataProvider::parseFileDetailsData( XFile* file, QIODevice &stream )
{
    QString author, info;

    try {
        author = SDTP::unpackString(stream);
        info = SDTP::unpackString(stream);
    } catch( std::exception *e ) {
        qDebug("Failed to parse 'file_details' packet");
        delete e;
        return false;
    }

    file->setAuthor(author);
    file->setInfo(info);

    return true;
}

bool SdtpCatalogDataProvider::parseSearchData( QIODevice &stream, XFileList& list )
{
    while ( stream.bytesAvailable() > 0 ) {
        quint32 id, pid, date;
        QString name, thumbLink;

        try {
            id = SDTP::unpackUInt32(stream);
            pid = SDTP::unpackUInt32(stream);
            name = SDTP::unpackString(stream);
            thumbLink = SDTP::unpackString(stream);
            date = SDTP::unpackUInt32(stream);
        } catch( std::exception *e ) {
            qDebug("Failed to parse 'search' packet");
            delete e;
            for ( XFileList::iterator it = list.begin(); it != list.end(); ++it )
                delete *it;
            return false;
        }

        XFile *file = new XFile(id, NULL, name);
        file->setThumbLink(thumbLink);
        file->setDate(date);
        file->setPid(pid);

        list.append(file);
    }

    return true;
}

#ifdef QT_DEBUG
void SdtpCatalogDataProvider::debug_addRequestTime( QNetworkReply *reply )
{
    QTime timer;
    timer.start();
    m_debug_requestStart[reply] = timer;
}
#endif
