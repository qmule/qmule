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

#include <QtCore/QThread>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QDesktopServices>
#include <QtGui/QPainter>
#include "config.h"
#include "constants.h"
#include "catalog.h"
#include "sdtpcatalogdataprovider.h"

XCatalog::XCatalog( QObject *parent ) :
    QObject(parent),
    m_dataProvider(new SdtpCatalogDataProvider),
    m_dataProviderThread(new QThread),
    m_rootFolder(new XFolder(0,QLatin1String("_root_"),1,0)),
    m_selectedFolder(NULL),
    m_selectedFile(NULL)
{
    qRegisterMetaType< XFolderList >("XFolderList");
    qRegisterMetaType< XFileList >("XFileList");
    qRegisterMetaType< XFolderDict >("XFolderDict");

//    m_dataProvider->moveToThread2(m_dataProviderThread);
//    m_dataProviderThread->start();

    connect(m_dataProvider, SIGNAL(data(int,void*,void*)),
            this, SLOT(onDataFetched(int,void*,void*)));

    connect(m_dataProvider, SIGNAL(error(int,void*)),
            this, SLOT(onError(int,void*)) );

    connect(this, SIGNAL(queueSearch(QString,XFolderList)),
            m_dataProvider, SLOT(search(QString,XFolderList)) );

    connect(this, SIGNAL(queueFetch(int,void*)),
            m_dataProvider, SLOT(fetch(int,void*)) );

    m_imageCachePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QLatin1String("/images");
    m_imageCachePath = QDir::toNativeSeparators(m_imageCachePath);
}

XCatalog::~XCatalog()
{
    m_dataProvider->deleteLater();
    m_dataProviderThread->deleteLater();
    delete m_rootFolder;
}

void XCatalog::fetchFoldersRecursive( XFolder *parent )
{
    if( Done == parent->foldersFetchStatus() ) {
        emit foldersFetched(parent);
    } else if (None == parent->foldersFetchStatus() ) {
        parent->setFoldersFetchStatus(Fetching);
        emit queueFetch(FoldersRecursive, parent);
    }
}

void XCatalog::fetchFolders( XFolder *parent )
{
    if( Done == parent->foldersFetchStatus() ) {
        emit foldersFetched(parent);
    } else if (None == parent->foldersFetchStatus() ) {
        parent->setFoldersFetchStatus(Fetching);
        emit queueFetch(Folders, parent);
    }
}

void XCatalog::fetchFiles( XFolder *parent )
{
    if( Done == parent->filesFetchStatus() ) {
        emit filesFetched(parent);
    } else if ( None == parent->filesFetchStatus() ) {
        parent->setFilesFetchStatus(Fetching);
        emit queueFetch(Files, parent);
    }
}

void XCatalog::fetchFileDetails( XFile *file )
{
    if( !file->info().isEmpty() ) {
        emit fileDetailsFetched(file);
    } else {
        emit queueFetch(FileDetails, file);
    }
}

void XCatalog::fetchThumbnail( XFile *file )
{
    // check for disk cache
    QString imgFile = cachedThumbnail(file);

    if( QFile::exists(imgFile) &&
        QFileInfo(imgFile).lastModified().daysTo(QDateTime::currentDateTime()) < XCFG_THUMB_REFRESH_TIME )
    {
        file->setThumbnail( QImage(imgFile, XCFG_THUMB_FORMAT) );
        file->setThumbFetchStatus(Done);
        emit thumbnailFetched(file);
    } else {
        file->setThumbFetchStatus(Fetching);
        emit queueFetch(Thumbnail, file);
    }
}

void XCatalog::onDataFetched(int type, void *param, void *data)
{
    switch(type)
    {
    case FoldersRecursive:
    {
        XFolder *parent = static_cast<XFolder*>(param);
        XFolderList *list = static_cast<XFolderList*>(data);
        parent->setFolderList(*list);
        delete list;

        break;
    }

    case FoldersRecursiveDictionary:
    {
        const XFolder *parent = static_cast<XFolder*>(param);
        XFolderDict *dict = static_cast<XFolderDict*>(data);

        // we need dictionary only for root folder
        if( parent == m_rootFolder ) {
            m_folderDict = *dict;
            m_rootFolder->setFoldersFetchStatus(Done);
            emit allFoldersFetched(m_rootFolder);
        }
        delete dict;

        break;
    }

    case Folders:
    {
        XFolder *parent = static_cast<XFolder*>(param);
        XFolderList *list = static_cast<XFolderList*>(data);

        parent->setFolderList(*list);
        delete list;
        parent->setFoldersFetchStatus(Done);
        emit foldersFetched(parent);

        break;
    }

    case Files:
    {
        XFolder *parent = static_cast<XFolder*>(param);
        XFileList *list = static_cast<XFileList*>(data);

        parent->setFileList(*list);
        delete list;
        parent->setFilesFetchStatus(Done);
        emit filesFetched(parent);

        break;
    }

    case FileDetails:
    {
        XFile *file = static_cast<XFile*>(param);
        XFile *dummy = static_cast<XFile*>(data);

        file->setAuthor(dummy->author());
        file->setInfo(dummy->info());
        delete dummy;

        emit fileDetailsFetched(file);

        break;
    }

    case Thumbnail:
    {
        XFile *file = static_cast<XFile*>(param);
        QImage *image = static_cast<QImage*>(data);

        if ( (NULL != m_selectedFolder) && (-1 != m_selectedFolder->files().indexOf(file)) ) {
            file->setThumbnail( createThumbnail(*image) );
            file->setThumbFetchStatus(Done);

            if ( file->thumbnail().isNull() )
                return;

            // store to cache
            QDir dirHelper;
            if( !dirHelper.exists(m_imageCachePath) ) {
                dirHelper.mkpath(m_imageCachePath);
            }

            QString imgFile = cachedThumbnail(file);
            if ( !file->thumbnail().save(imgFile, XCFG_THUMB_FORMAT) ) {
                qDebug("unable to save thumbnail");
            }

            emit thumbnailFetched(file);
        }
        delete image;

        break;
    }


    case Search:
    {
        XFileList *list = static_cast<XFileList*>(data);

        XFolder *dummySearch = new XFolder(0, QString("_search_"), 0, 0);
        for ( XFileList::iterator it = list->begin();
              it != list->end(); ++it ) {
            XFile* file = *it;
            XFolder* realParent = m_folderDict[file->pid()];
            if ( !realParent )
                continue;
            file->setParent(realParent);
            dummySearch->addFile(file);
        }
        delete list;
        emit searchComplete(dummySearch);

        break;
    }

    default:
        qDebug("Unknown data type returned '%d'", type);
    }
}

void XCatalog::execSearch( QString query, XFolderList selectedFolders )
{
    emit queueSearch(query, selectedFolders);
}

void XCatalog::reset()
{
    m_rootFolder->clearFiles();
    m_rootFolder->clearFolders();
    m_selectedFolder = NULL;
    m_selectedFile = NULL;
    m_folderDict.clear();
}

void XCatalog::onError(int type, void *param)
{
    //clean some shit

    switch(type)
    {
    case FoldersRecursive:
    {
        XFolder *parent = static_cast<XFolder*>(param);
        parent->setFoldersFetchStatus(None);

        break;
    }

    case FoldersRecursiveDictionary:
    {
        break;
    }

    case Folders:
    {
        XFolder *parent = static_cast<XFolder*>(param);
        parent->setFoldersFetchStatus(None);

        break;
    }

    case Files:
    {
        XFolder *parent = static_cast<XFolder*>(param);
        parent->setFilesFetchStatus(None);

        break;
    }

    case FileDetails:
    {
        break;
    }

    case Thumbnail:
    {
        XFile *file = static_cast<XFile*>(param);
        file->setThumbFetchStatus(None);

        return;
    }

    case Search:
    {
        break;
    }

    default:
        qDebug("Unknown data type returned '%d'", type);
    }

    emit error(type, param);
}

QString XCatalog::cachedThumbnail( const XFile *file ) const
{
    // TODO: this is bad code
    return QDir::toNativeSeparators( QString("%1/%2.%3")
                                     .arg(m_imageCachePath)
                                     .arg(file->id())
                                     .arg(XCFG_THUMB_FORMAT) );
}

QImage XCatalog::createThumbnail( const QImage& image )
{
    // prepare thumbnail
    if( image.size().isEmpty() ) {
        qDebug("THUMBNAILER: empty image.");
        return QImage();
    }

    // prepare
    QImage bg(QSize(XCFG_THUMB_WIDTH, XCFG_THUMB_HEIGHT), QImage::Format_ARGB32 );
    QImage thumb = image.scaled(QSize(XCFG_THUMB_WIDTH, XCFG_THUMB_HEIGHT),
                                Qt::KeepAspectRatio, Qt::SmoothTransformation );

    // calculate center position of image
    QRect rc = thumb.rect();
    rc.moveCenter(bg.rect().center());

    QPainter painter(&bg);
    // draw bg
    painter.eraseRect(bg.rect());
    // draw image
    painter.drawImage(rc.topLeft(), thumb);
    // draw border
    painter.setBrush( Qt::NoBrush );
    painter.setPen( QColor(0xCC, 0xCC, 0xCC) );
    rc = bg.rect();
    rc.setHeight(rc.height()-1);
    rc.setWidth(rc.width()-1);
    painter.drawRect( rc );

    return bg;
}

QString XCatalog::createFolderPath( const XFolder *folder ) const
{
    QString path = folder->name();
    folder = folder->parent();
    while ( (NULL != folder) && (0 != folder->id()) ) {
        path.prepend(folder->name() + " / ");
        folder = folder->parent();
    }
    return path;
}
