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

#ifndef TOOLS_H
#define TOOLS_H

namespace Util {

void openUrl( const QString& url );

QString readFileContents(const QString &filepath, const char *cp);
bool isMovie( const QString &fileName );

} // namespace Util

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

#endif // TOOLS_H
