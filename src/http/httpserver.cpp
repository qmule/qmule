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

HttpServer::HttpServer(QObject* parent):
    QTcpServer(parent), m_sessionsCount(0)
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
    //connect(this, SIGNAL(interrupt()), conn, SLOT(interrupt()));
    conn->moveToThread(thread);
    thread->start();
}

bool HttpServer::allocateSession()
{
    Preferences pref;
    bool res = false;

    if (m_sessionsCount < pref.httpSesLimit())
    {
        ++m_sessionsCount;
        res = true;
    }

    return res;
}

void HttpServer::freeSession()
{
    if (m_sessionsCount > 0)
    {
        --m_sessionsCount;
    }
    else
    {
        qWarning() << "Nothing to free, current session count is: " << m_sessionsCount;
    }
}
