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

#include <QtCore/QtGlobal>
#include "folder.h"

XFile::XFile( int id, XFolder *parent, const QString &name) :
    m_id(id),
    m_pid(0),
    m_parent(parent),
    m_name(name),
    m_thumbFetchStatus(None)
{
}

XFolder::XFolder( int id, const QString &name, int children, XFolder *parent ) :
    m_id(id),
    m_children(children),
    m_name(name),
    m_parent(parent),
    m_filesFetchStatus(None),
    m_foldersFetchStatus(None)
{
}

XFolder::~XFolder()
{
    clearFolders();
    clearFiles();
}

void XFolder::clearFiles()
{
    for ( XFileList::iterator it = m_files.begin(); it != m_files.end(); ++it ) {
        delete *it;
    }
    m_files.clear();
    m_filesFetchStatus = None;
}

void XFolder::clearFolders()
{
    for ( XFolderList::iterator it = m_folders.begin(); it != m_folders.end(); ++it ) {
        delete *it;
    }
    m_folders.clear();
    m_foldersFetchStatus = None;
}

void XFolder::freeThumbnails()
{
    for ( XFileList::iterator it = m_files.begin(); it != m_files.end(); ++it ) {
        (*it)->freeThumbnail();
    }
}
