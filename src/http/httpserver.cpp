#include "httpserver.h"
#include "httpconnection.h"
#include "preferences.h"

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

HttpServer::HttpServer(QObject* parent): QTcpServer(parent)
{
}

HttpServer::~HttpServer() {}

void HttpServer::incomingConnection(int socketDescriptor)
{
    HttpConnection* conn = new HttpConnection(this, socketDescriptor);
    QThread* thread = new QThread(this);

    connect(thread, SIGNAL(started()), conn, SLOT(start()));
    connect(conn, SIGNAL(finished()), thread, SLOT(quit()));
    connect(conn, SIGNAL(finished()), conn, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    conn->moveToThread(thread);
    thread->start();
}

void HttpServer::stop(bool disconnectClients)
{
    close(); // stop server listening

    if (disconnectClients)
    {
        m_connectionsMutex.lock();
        foreach(HttpConnection* c, m_connections)
            c->interrupt();
        m_connectionsMutex.unlock();
    }
}

bool HttpServer::registerConnection(HttpConnection* c)
{
    Preferences pref;
    bool res = false;

    m_connectionsMutex.lock();
    if (m_connections.size() < pref.httpSesLimit())
    {
        m_connections.insert(c);
        res = true;
    }
    m_connectionsMutex.unlock();

    return res;
}

void HttpServer::unregisterConnection(HttpConnection* c)
{
    m_connectionsMutex.lock();
    m_connections.remove(c);
    m_connectionsMutex.unlock();
}
