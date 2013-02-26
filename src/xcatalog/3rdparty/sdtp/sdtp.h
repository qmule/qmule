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

#ifndef SDTP_H
#define SDTP_H

#include <QtCore/QDataStream>

namespace SDTP {

enum Type {
    TAG_UINT32 = 0x01,
    TAG_STRING
};

quint32 unpackUInt32( QIODevice &stream );
QString unpackString( QIODevice &stream );

}

#endif // SDTP_H
