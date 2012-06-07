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

#ifndef CONFIG_H
#define CONFIG_H

#define XCFG_APPNAME "xCatalog"
#define XCFG_APPNAME_CFG "eMule IS Mod"
#define XCFG_VERSION "0.06"

#define XCFG_THUMB_FORMAT "jpg" // thumbnail file extension
#define XCFG_THUMB_WIDTH 50
#define XCFG_THUMB_HEIGHT 50
#define XCFG_THUMB_REFRESH_TIME 5 // in days

#ifdef _WINDLL
#define XCFG_POSTFIX "_dll"
#else
#define XCFG_POSTFIX
#endif

#define XCFG_WINDOW_GEOMETRY  "xcat_geometry" XCFG_POSTFIX
#define XCFG_WINDOW_STATE     "xcat_state" XCFG_POSTFIX
#define XCFG_FILES_SORT_ROLE  "xcat_files_sort_role" XCFG_POSTFIX
#define XCFG_FILES_SORT_ORDER "xcat_file_sort_order" XCFG_POSTFIX




#endif // CONFIG_H
