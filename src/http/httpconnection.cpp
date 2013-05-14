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

#include "httpconnection.h"
#include "httpserver.h"
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

/*
void HttpConnection::translateDocument(QString& data) {
  static QRegExp regex(QString::fromUtf8("_\\(([\\w\\s?!:\\/\\(\\),%µ&\\-\\.]+)\\)"));
  static QRegExp mnemonic("\\(?&([a-zA-Z]?\\))?");
  const std::string contexts[] = {"TransferListFiltersWidget", "TransferListWidget",
                                  "PropertiesWidget", "MainWindow", "HttpServer",
                                  "confirmDeletionDlg", "TrackerList", "TorrentFilesModel",
                                  "options_imp", "Preferences", "TrackersAdditionDlg",
                                  "ScanFoldersModel", "PropTabBar", "TorrentModel",
                                  "downloadFromURL"};
  int i = 0;
  bool found;

  do {
    found = false;

    i = regex.indexIn(data, i);
    if (i >= 0) {
      //qDebug("Found translatable string: %s", regex.cap(1).toUtf8().data());
      QByteArray word = regex.cap(1).toUtf8();

      QString translation = word;
      bool isTranslationNeeded = !Preferences().getLocale().startsWith("en");
      if (isTranslationNeeded) {
        int context_index = 0;
        do {
          translation = qApp->translate(contexts[context_index].c_str(), word.constData(), 0, QCoreApplication::UnicodeUTF8, 1);
          ++context_index;
        } while(translation == word && context_index < 15);
      }
      // Remove keyboard shortcuts
      translation.replace(mnemonic, "");

      data.replace(i, regex.matchedLength(), translation);
      i += translation.length();
      found = true;
    }
  } while(found && i < data.size());
}
*/

void HttpConnection::respond()
{
    QString url  = m_parser.url();

    // Favicon
    if (url.endsWith("favicon.ico"))
    {
        qDebug("Returning favicon");
        QFile favicon(":/Icons/skin/qbittorrent16.png");

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
            respondNotFound();
        }

        return;
    }

    QStringList list = url.split('/', QString::SkipEmptyParts);

    if (list.contains(".") || list.contains(".."))
    {
        respondNotFound();
        return;
    }

  if (list.isEmpty())
    list.append("index.html");

  /*
  if (list.size() >= 2) {
    if (list[0] == "json") {
      if (list[1] == "torrents") {
        respondTorrentsJson();
        return;
      }
      if (list.size() > 2) {
        if (list[1] == "propertiesGeneral") {
          const QString& hash = list[2];
          respondGenPropertiesJson(hash);
          return;
        }
        if (list[1] == "propertiesTrackers") {
          const QString& hash = list[2];
          respondTrackersPropertiesJson(hash);
          return;
        }
        if (list[1] == "propertiesFiles") {
          const QString& hash = list[2];
          respondFilesPropertiesJson(hash);
          return;
        }
      } else {
        if (list[1] == "preferences") {
          respondPreferencesJson();
          return;
        } else {
          if (list[1] == "transferInfo") {
            respondGlobalTransferInfoJson();
            return;
          }
        }
      }
    }


    if (list[0] == "command") {
      const QString& command = list[1];
      if (command == "shutdown") {
        qDebug() << "Shutdown request from Web UI";
        // Special case handling for shutdown, we
        // need to reply to the Web UI before
        // actually shutting down.
        m_generator.setStatusLine(200, "OK");
        finish();
        qApp->processEvents();
        // Exit application
        qApp->exit();
      } else {
        respondCommand(command);
        m_generator.setStatusLine(200, "OK");
        finish();
      }
      return;
    }
  }
*/
  /*
  // Icons from theme
  //qDebug() << "list[0]" << list[0];
  if (list[0] == "theme" && list.size() == 2) {
#ifdef DISABLE_GUI
    url = ":/Icons/oxygen/"+list[1]+".png";
#else
    url = IconProvider::instance()->getIconPath(list[1]);
#endif
    qDebug() << "There icon:" << url;
  } else {
    if (list[0] == "images") {
      list[0] = "Icons";
    } else {
      if (list.last().endsWith(".html"))
        list.prepend("html");
      list.prepend("webui");
    }
    url = ":/" + list.join("/");
  }

  QFile file(url);

  if (!file.open(QIODevice::ReadOnly))
  {
    qDebug("File %s was not found!", qPrintable(url));
    respondNotFound();
    return;
  }

  QString ext = list.last();
  int index = ext.lastIndexOf('.') + 1;
  if (index > 0)
    ext.remove(0, index);
  else
    ext.clear();
  QByteArray data = file.readAll();
  file.close();

  // Translate the page
  if (ext == "html" || (ext == "js" && !list.last().startsWith("excanvas")))
  {
    QString dataStr = QString::fromUtf8(data.constData());
    translateDocument(dataStr);
    if (url.endsWith("about.html")) {
      dataStr.replace("${VERSION}", VERSION);
    }
    data = dataStr.toUtf8();
  }
*/

    m_generator.setStatusLine(200, "OK");
    //m_generator.setContentTypeByExt(ext);
    m_generator.setMessage(QString("<html><body><h3>Tentative page from qmule</h3></body></html>"));
    finish();
}

void HttpConnection::respondNotFound()
{
    m_generator.setStatusLine(404, "File not found");
    finish();
}

// respond examples
/*
void HttpConnection::respondTorrentsJson() {
  m_generator.setStatusLine(200, "OK");
  m_generator.setContentTypeByExt("js");
  m_generator.setMessage(btjson::getTorrents());
  finish();
}

void HttpConnection::respondGenPropertiesJson(const QString& hash) {
  m_generator.setStatusLine(200, "OK");
  m_generator.setContentTypeByExt("js");
  m_generator.setMessage(btjson::getPropertiesForTorrent(hash));
  finish();
}

void HttpConnection::respondTrackersPropertiesJson(const QString& hash) {
  m_generator.setStatusLine(200, "OK");
  m_generator.setContentTypeByExt("js");
  m_generator.setMessage(btjson::getTrackersForTorrent(hash));
  finish();
}

void HttpConnection::respondFilesPropertiesJson(const QString& hash) {
  m_generator.setStatusLine(200, "OK");
  m_generator.setContentTypeByExt("js");
  m_generator.setMessage(btjson::getFilesForTorrent(hash));
  finish();
}

void HttpConnection::respondPreferencesJson() {
  m_generator.setStatusLine(200, "OK");
  m_generator.setContentTypeByExt("js");
  m_generator.setMessage(prefjson::getPreferences());
  finish();
}

void HttpConnection::respondGlobalTransferInfoJson() {
  m_generator.setStatusLine(200, "OK");
  m_generator.setContentTypeByExt("js");
  m_generator.setMessage(btjson::getTransferInfo());
  finish();
}
*/
