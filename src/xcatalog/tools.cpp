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

#include <QtGui/QDesktopServices>
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtCore/QStringList>

#ifdef Q_OS_WIN
#include <shlwapi.h>
#include <ShellAPI.h>
#endif

#include "tools.h"

namespace Util {

void openUrl( const QString& url )
{
#ifdef Q_OS_WIN
    size_t len = url.length();
    wchar_t *wurl = (wchar_t*)malloc( (len+1)*sizeof(wchar_t) );
    url.toWCharArray(wurl);
    wurl[len] = L'\0';
    ShellExecuteW(0, L"open", wurl, 0,0, SW_SHOWMAXIMIZED);
    free(wurl);
#else
    QDesktopServices::openUrl(url);
#endif
}

QString readFileContents( const QString &filepath, const char *cp )
{
    QFile file(filepath);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QTextStream stream(&file);
        stream.setCodec(cp);
        return stream.readAll();
    } else {
        return QString();
    }
}

bool isMovie( const QString &fileName )
{
    static QString moviesExt = QLatin1String(".3g2 .3gp .3gp2 .3gpp .amv .asf .avi .bik .divx .dvr-ms .flc .fli .flic "
        ".flv .hdmov .ifo .m1v .m2t .m2ts .m2v .m4b .m4v .mkv .mov .movie .mp1v .mp2v .mp4 .mpe .mpeg "
        ".mpg .mpv .mpv1 .mpv2 .ogm .pva .qt .ram .ratdvd .rm .rmm .rmvb .rv .smil .smk .swf .tp .ts "
        ".vid .video .vob .vp6 .wm .wmv .xvid ");

    int dotIndex = fileName.lastIndexOf('.');

    QString ext = fileName.mid(dotIndex);

    if ( ext.length() <= 1 )
        return false;

    ext.append(' ');

    return moviesExt.indexOf(ext) != -1;
}

} // namspace Util
