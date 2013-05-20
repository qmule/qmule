
#ifndef __HTTPTRANSFER__
#define __HTTPTRANSFER__

#include <QRunnable>
#include <QString>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

class HttpTransfer : public QRunnable
{
public:
    HttpTransfer(const QString& srcPath, QTcpSocket* dst);

    void run();
    void interrupt();

private:
    QString contentType();

    QString m_srcPath;
    QTcpSocket* m_dst;
    volatile bool m_interrupted;
};

#endif
