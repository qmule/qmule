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

#ifndef FOLDER_H
#define FOLDER_H

#include <QtCore/QMetaType>
#include <QtCore/QDateTime>
#include <QtCore/QVector>
#include <QtCore/QHash>
#include <QtGui/QImage>
#include "constants.h"

class XFolder;
class XFile;

typedef QVector<XFolder*> XFolderList;
typedef QVector<XFile*> XFileList;
typedef QHash<quint32,XFolder*> XFolderDict;

enum SortParam {
    SortByName = 0,
    SortByDate
};

class XFile
{
    Q_DISABLE_COPY(XFile)

public:
    explicit XFile( int id, XFolder* parent, const QString& name);

    inline quint32 id() const { return m_id; }
    inline const QString& name() const { return m_name; }

    inline quint32 pid() const { return m_pid; }
    inline void setPid( quint32 pid ) { m_pid = pid; }

    inline const XFolder* parent() const { return m_parent; }
    inline void setParent( XFolder* parent ) { m_parent = parent; }

    inline const QString& info() const { return m_info; }
    inline void setInfo(const QString& info ) { m_info = info; }

    inline const QString& author() const { return m_author; }
    inline void setAuthor(const QString& author ) { m_author = author; }

    inline const QImage& thumbnail() const { return m_thumbImage; }
    inline void setThumbnail(const QImage& image ) { m_thumbImage = image; }
    inline FetchStatus thumbFetchStatus() const { return m_thumbFetchStatus; }
    inline void setThumbFetchStatus( FetchStatus status ) { m_thumbFetchStatus = status; }

    inline const QString& thumbLink() const { return m_thumbLink; }
    inline void setThumbLink( const QString& thumbLink ) { m_thumbLink = thumbLink; }

    inline void freeThumbnail() {
        m_thumbImage = QImage();
        m_thumbFetchStatus = None;
    }

    inline const QDateTime& date() const { return m_date; }
    inline void setDate( uint date ) { m_date.setTime_t(date); }
    inline void setDate( const QDateTime& date ) { m_date = date; }

    static bool lessThanName( const XFile* f1, const XFile* f2 );
    static bool moreThanName( const XFile* f1, const XFile* f2 );
    static bool lessThanDate( const XFile* f1, const XFile* f2 );
    static bool moreThanDate( const XFile* f1, const XFile* f2 );

private:
    quint32 m_id;
    quint32 m_pid;
    XFolder* m_parent;
    QString m_name;
    QString m_info;
    QString m_author;
    QString m_thumbLink;
    QImage m_thumbImage;
    FetchStatus m_thumbFetchStatus;
    QDateTime m_date;
};

Q_DECLARE_METATYPE( XFile * );

class XFolder {
    Q_DISABLE_COPY(XFolder)

public:
    explicit XFolder( int id, const QString& name, int children, XFolder* parent );
    virtual ~XFolder();

    // own stuff
    inline quint32 id() const { return m_id; }
    inline bool hasChildren() const { return m_children > 0; }
    inline const XFolder* parent() const { return m_parent; }
    inline const QString& name() const { return m_name; }

    // files stuff
    inline const XFileList& files() const { return m_files; }
    inline void addFile( XFile *file ) { m_files.append(file); }
    inline void setFileList( const XFileList& list ) { m_files = list; }
    inline FetchStatus filesFetchStatus() const { return m_filesFetchStatus; }
    inline void setFilesFetchStatus( FetchStatus status ) { m_filesFetchStatus = status; }
    void sortFiles( SortParam sortParam, Qt::SortOrder order );

    // child folders stuff
    inline const XFolderList& folders() const { return m_folders; }
    inline void addFolder( XFolder* folder ) { m_folders.append(folder); }
    inline void setFolderList( const XFolderList& list ) { m_folders = list; }
    inline FetchStatus foldersFetchStatus() const { return m_foldersFetchStatus; }
    inline void setFoldersFetchStatus( FetchStatus status ) { m_foldersFetchStatus = status; }

    void clearFiles();
    void clearFolders();
    void freeThumbnails();

private:
    quint32 m_id;
    quint32 m_children;
    QString m_name;
    XFolder* m_parent;
    XFolderList m_folders;
    XFileList m_files;
    FetchStatus m_filesFetchStatus;
    FetchStatus m_foldersFetchStatus;
};

#endif // FOLDER_H
