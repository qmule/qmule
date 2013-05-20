
#ifndef __HTTPTRANSFER__
#define __HTTPTRANSFER__

#include <QRunnable>
#include <QString>

class HttpServer;

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

class HttpTransfer : public QRunnable
{
public:
    HttpTransfer(HttpServer* httpServer, const QString& srcPath, QTcpSocket* dst);
    ~HttpTransfer();

    void run();
    void interrupt();

private:
    QString contentType();

    HttpServer* m_httpServer;
    QString m_srcPath;
    QTcpSocket* m_dst;
    volatile bool m_interrupted;
};

#endif
