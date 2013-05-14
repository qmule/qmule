#include "httpserver.h"
#include "httpconnection.h"

#include <QCryptographicHash>
#include <QTime>
#include <QRegExp>
#include <QTimer>
#include <QTcpSocket>

const int BAN_TIME = 3600000; // 1 hour

class UnbanTimer: public QTimer {
public:
  UnbanTimer(const QString& peer_ip, QObject *parent): QTimer(parent),
    m_peerIp(peer_ip) {
    setSingleShot(true);
    setInterval(BAN_TIME);
  }

  inline const QString& peerIp() const { return m_peerIp; }

private:
  QString m_peerIp;
};

HttpServer::HttpServer(QObject* parent) : QTcpServer(parent)
{
  // Additional translations for Web UI
  QString a = tr("File");
  a = tr("Edit");
  a = tr("Help");
  a = tr("Download Torrents from their URL or Magnet link");
  a = tr("Only one link per line");
  a = tr("Download local torrent");
  a = tr("Torrent files were correctly added to download list.");
  a = tr("Point to torrent file");
  a = tr("Download");
  a = tr("Are you sure you want to delete the selected torrents from the transfer list and hard disk?");
  a = tr("Download rate limit must be greater than 0 or disabled.");
  a = tr("Upload rate limit must be greater than 0 or disabled.");
  a = tr("Maximum number of connections limit must be greater than 0 or disabled.");
  a = tr("Maximum number of connections per torrent limit must be greater than 0 or disabled.");
  a = tr("Maximum number of upload slots per torrent limit must be greater than 0 or disabled.");
  a = tr("Unable to save program preferences, qBittorrent is probably unreachable.");
  a = tr("Language");
  a = tr("Downloaded", "Is the file downloaded or not?");
  a = tr("The port used for incoming connections must be greater than 1024 and less than 65535.");
  a = tr("The port used for the Web UI must be greater than 1024 and less than 65535.");
  a = tr("The Web UI username must be at least 3 characters long.");
  a = tr("The Web UI password must be at least 3 characters long.");
  a = tr("Save");
  a = tr("qBittorrent client is not reachable");
  a = tr("HTTP Server");
  a = tr("The following parameters are supported:");
  a = tr("Torrent path");
  a = tr("Torrent name");
  a = tr("qBittorrent has been shutdown.");
}

HttpServer::~HttpServer() {}

void HttpServer::incomingConnection(int socketDescriptor)
{
    QTcpSocket *serverSocket = new QTcpSocket(this);

    if (serverSocket->setSocketDescriptor(socketDescriptor))
    {
        // check IP, count
        handleNewConnection(serverSocket);
    }
    else
    {
        serverSocket->deleteLater();
    }
}

void HttpServer::handleNewConnection(QTcpSocket *socket)
{
    HttpConnection *connection = new HttpConnection(socket, this);
}
