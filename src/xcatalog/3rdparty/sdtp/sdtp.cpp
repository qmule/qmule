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

#include "sdtp.h"
#include <stdexcept>

namespace SDTP {

quint32 unpackUInt32( QIODevice &stream )
{
    if ( stream.bytesAvailable() < 5 ) {
        qDebug("Unexpected packet end at uint32 tag");
        throw new std::exception();
    }

    quint8 type;
    stream.read((char*)&type, sizeof(type));
    if ( TAG_UINT32 != type ) {
        qDebug("Invalid uint32 tag type, got %02x", type);
        throw new std::exception();
    }

    quint32 val;
    stream.read((char*)&val, sizeof(val));

    return val;
}

QString unpackString( QIODevice &stream )
{
    if ( stream.bytesAvailable() < 5 ) {
        qDebug("Unexpected packet end at string tag");
        throw new std::exception();
    }

    quint8 type;
    stream.read((char*)&type, sizeof(type));
    if ( TAG_STRING != type ) {
        qDebug("Invalid string tag type, got %02x", type);
        throw new std::exception();
    }

    quint32 length;
    stream.read((char*)&length, sizeof(length));
    if ( 10*1024*1024 < length ) {
        qDebug("String length %u, abort reading", length);
        throw new std::exception();
    }

    return QString::fromUtf8(stream.read(length).constData(), length);
}

}
