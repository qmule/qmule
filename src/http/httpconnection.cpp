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
#include "httptransfer.h"
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

HttpConnection::HttpConnection(QTcpSocket *socket, HttpServer *parent)
  : QObject(parent), m_socket(socket), m_httpserver(parent)
{
    m_socket->setParent(this);
    connect(m_socket, SIGNAL(readyRead()), SLOT(read()));
    connect(m_socket, SIGNAL(disconnected()), SLOT(deleteLater()));
}

HttpConnection::~HttpConnection()
{
    delete m_socket;
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
                                   "<title>qMule start page</title>"
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
                //m_generator.setStatusLine(200, "OK");
                //m_generator.setMessage(QString("<html><body><h3>ready to download</h3></body></html>"));
                //finish();
                HttpTransfer ht(t.absolute_files_path()[0], m_socket);
                ht.run();
                return;
            }
        }
    }

    respondNotFound();
}

void HttpConnection::respondNotFound()
{
    m_generator.setStatusLine(404, "File not found");
    finish();
}
