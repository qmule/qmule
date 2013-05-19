
#include "httptransfer.h"
#include <QTcpSocket>
#include <QFile>

HttpTransfer::HttpTransfer(const QString& srcPath, QTcpSocket* dst):
    m_srcPath(srcPath), m_dst(dst), m_interrupted(false)
{
}

void HttpTransfer::run()
{
  QFile srcFile(m_srcPath);
  QByteArray buf;

  if (srcFile.open(QIODevice::ReadOnly))
  {
    buf += "POST /push1_pub?id=game HTTP/1.1\r\n";
    buf += "User-Agent: qMule\r\n";
    buf += "Host: localhost\r\n";
    buf += "Accept: */*\r\n";
    buf += "Content-Type: application/octet-stream\r\n";
    buf += QString("Content-Length: %1\r\n\r\n").arg(srcFile.size()).toUtf8();

    if (m_dst->write(buf) != -1 && m_dst->waitForBytesWritten())
    {
      while(!m_interrupted && !srcFile.atEnd())
      {
        buf = srcFile.read(256 * 1024);
        if (buf.size() == 0)
        {
          qDebug() << "Error: cannot read file: " << m_srcPath;
          break;
        }
        if (m_dst->write(buf) == -1 || !m_dst->waitForBytesWritten())
        {
          qDebug() << "Error: cannot send file: " << m_srcPath;
          break;
        }
      }
    }
    else
      qDebug() << "Error: cannot send header: " << m_srcPath;

    if (srcFile.atEnd())
      qDebug() << "File upload completed: " << m_srcPath;

    srcFile.close();
    m_dst->disconnectFromHost();
  }
  else
    qDebug() << "Error: cannot open file: " << m_srcPath;
}

void HttpTransfer::interrupt()
{
  qDebug() << "Interrupt file transfer: " << m_srcPath;
  m_interrupted = true;
}
