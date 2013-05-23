/*
 * Bittorrent Client using Qt4 and libtorrent.
 * Copyright (C) 2006  Ishan Arora and Christophe Dumez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * Contact : chris@qbittorrent.org
 */

#include "transport/session.h"
#include "httpconnection.h"
#include "httpserver.h"
#include "misc.h"
#include <QTcpSocket>
#include <QDateTime>
#include <QStringList>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QFile>
#include <QDebug>
#include <QRegExp>
#include <QTemporaryFile>
#include <queue>
#include <vector>

HttpConnection::HttpConnection(HttpServer* httpserver, int socketDescriptor):
    m_httpserver(httpserver), m_socketDescriptor(socketDescriptor), m_interrupted(false)
{
}

void HttpConnection::interrupt()
{
    m_interrupted = true;
}

void HttpConnection::start()
{
    m_socket = new QTcpSocket(this);
    m_socket->setSocketDescriptor(m_socketDescriptor);
    connect(m_socket, SIGNAL(readyRead()), SLOT(read()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(finished()));
}

void HttpConnection::read()
{
    m_receivedData.append(m_socket->readAll());

    // Parse HTTP request header
    const int header_end = m_receivedData.indexOf("\r\n\r\n");

    if (header_end < 0)
    {
      qDebug() << "Partial request: \n" << m_receivedData;
      return;
    }

    const QByteArray header = m_receivedData.left(header_end);
    m_parser.writeHeader(header);

    if (m_parser.isError())
    {
        qWarning() << Q_FUNC_INFO << "header parsing error";
        m_receivedData.clear();
        m_generator.setStatusLine(400, "Bad Request");
        finish();
    }
    else if (m_parser.header().hasContentLength())  // Parse HTTP request message
    {
        const int expected_length = m_parser.header().contentLength();
        QByteArray message = m_receivedData.mid(header_end + 4, expected_length);

        if (expected_length > 10000000 /* ~10MB */)
        {
            qWarning() << "Bad request: message too long";
            m_generator.setStatusLine(400, "Bad Request");
            m_receivedData.clear();
            finish();
        }
        else if (message.length() < expected_length)
        {
            // Message too short, waiting for the rest
            qDebug() << "Partial message:\n" << message;
        }
        else
        {
            m_parser.writeMessage(message);
            m_receivedData = m_receivedData.mid(header_end + 4 + expected_length);
        }
    }
    else
    {
        m_receivedData.clear();
    }

    if (m_parser.isError())
    {
        qWarning() << Q_FUNC_INFO << "message parsing error";
        m_generator.setStatusLine(400, "Bad Request");
        finish();
    }
    else
    {
        respond();
    }
}

void HttpConnection::finish()
{
    m_socket->write(m_generator.toByteArray());
    m_socket->disconnectFromHost();
}

void HttpConnection::respond()
{
    Preferences pref;
    QString url  = m_parser.url();    
    qDebug() << "URL: " << url;

    foreach(QString key, m_parser.getPairs().keys())
    {
        qDebug() << "Pair:" << key << "/" << m_parser.get(key);
    }

    // Favicon
    if (url.endsWith("favicon.ico"))
    {
        qDebug("Returning favicon");
        QFile favicon(":/emule/newmule16.png");

        if (favicon.open(QIODevice::ReadOnly))
        {
            const QByteArray data = favicon.readAll();
            favicon.close();
            m_generator.setStatusLine(200, "OK");
            m_generator.setContentTypeByExt("png");
            m_generator.setMessage(data);
            finish();
        }
        else
        {
            qWarning() << "can't open icon for favicon";
            respondNotFound();
        }
    }
    else
    {
        QStringList list = url.split('/', QString::SkipEmptyParts);

        if (list.contains(".") || list.contains(".."))
        {
            respondNotFound();
            return;
        }

        if (list.isEmpty())
        {
            // display main page
            m_generator.setStatusLine(200, "OK");
            //m_generator.setContentTypeByExt(ext);
            m_generator.setMessage("<html><head>"
                                   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
                                   "<title>" + tr("Video files from: %1").arg(pref.nick()) +  + "</title>"
                                  "</head><body>"
                                  "<style>"
                                    "li {"
                                     "list-style-type: none; /* Убираем маркеры */"
                                   "}"
                                    "ul {"
                                     "margin-left: 4; /* Отступ слева в браузере IE и Opera */"
                                     "padding-left: 4; /* Отступ слева в браузере Firefox, Safari, Chrome */"
                                     "margin-top: 4;"
                                     "padding-top: 4;"
                                   "}"
                                    "li.marked {"
                                     "list-style-type: disc;"
                                     "margin-left: 20;"
                                   "}"
                                   "</style>"
                                   + Session::instance()->root()->toHtml(m_socket->localAddress().toString(), m_httpserver->serverPort()) + "</body></html>");
            finish();
            return;
        }


        if (misc::isMD4Hash(list[0]))
        {
            Transfer t = Session::instance()->getTransfer(list[0]);

            if (t.is_valid())
            {
                uploadFile(t.absolute_files_path()[0]);
                return;
            }
        }
    }

    respondNotFound();
}

void HttpConnection::uploadFile(const QString& srcPath)
{
    if (!m_httpserver->registerConnection(this))
    {
        respondLimitExceeded();
        return;
    }

    QFile srcFile(srcPath);
    QByteArray buf;
    if (srcFile.open(QIODevice::ReadOnly))
    {
        buf += "HTTP/1.1 200 OK\r\n";
        buf += QString("Content-Disposition: inline; filename=\"%1\"").arg(misc::fileName(srcPath)).toUtf8();
        buf += QString("Content-Type: %1\r\n").arg(contentType(srcPath)).toUtf8();
        buf += QString("Content-Length: %1\r\n\r\n").arg(srcFile.size()).toUtf8();

        if (m_socket->write(buf) != -1 && m_socket->waitForBytesWritten())
        {
            qDebug() << "Start uploading file: " << srcPath;
            while(true)
            {
                if (m_interrupted)
                {
                    qDebug() << "Interrupted uploading file: " << srcPath;
                    break;
                }

                buf = srcFile.read(256 * 1024);
                if (buf.size() == 0) break;
                if (m_socket->write(buf) == -1 || !m_socket->waitForBytesWritten())
                {
                    qDebug() << "Error: cannot send file: " << srcPath;
                    break;
                }
            }
        }
        else
            qDebug() << "Error: cannot send header: " << srcPath;

        qDebug() << "File upload completed: " << srcPath;
        srcFile.close();
    }
    else
        qDebug() << "Error: cannot open file: " << srcPath;

    m_httpserver->unregisterConnection(this);
    m_socket->disconnectFromHost();
}

QString HttpConnection::contentType(const QString& srcPath)
{
    QString ext = misc::file_extension(srcPath).toUpper();
    QString type = "application/octet-stream";

    if (ext == "MP2" || ext == "MPA" || ext == "MPE" ||
        ext == "MPEG" || ext == "MPG" || ext == "MPV2") type = "video/mpeg";
    if (ext == "MOV" || ext == "QT") type = "video/quicktime";
    if (ext == "LSF" || ext == "LSX" || ext == "ASF" || ext == "ASR" || ext == "ASX") type = "video/x-la-asf";
    if (ext == "AVI") type = "video/x-msvideo";
    if (ext == "MOVIE") type = "video/x-sgi-movie";

    if (ext == "AU" || ext == "SND") type = "audio/basic";
    if (ext == "MID" || ext == "RMI") type = "audio/mid";
    if (ext == "MP3") type = "audio/mpeg";
    if (ext == "AIF" || ext == "AIFC" || ext == "AIFF") type = "audio/x-aiff";
    if (ext == "M3U") type = "audio/x-mpegurl";
    if (ext == "RA" || ext == "RAM") type = "audio/x-pn-realaudio";
    if (ext == "WAV") type = "audio/x-wav";

    return type;
}

void HttpConnection::respondNotFound()
{
    m_generator.setStatusLine(404, "File not found");
    finish();
}

void HttpConnection::respondLimitExceeded()
{
    Preferences pref;
    m_generator.setStatusLine(503, "Connection limit exceeded");
    m_generator.setMessage("<html><head>"
                           "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
                           "<title>" + tr("Video files from: %1").arg(pref.nick()) +  + "</title>"
                           "</head><body><h3>" + tr("Connection limit exceeded") + "</h3></body></html>");
    finish();
}
