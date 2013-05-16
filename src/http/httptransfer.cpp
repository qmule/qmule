
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

    qDebug() << "File upload completed: " << m_srcPath;
    srcFile.close();
  }
  else
  {
    qDebug() << "Error: cannot open file: " << m_srcPath;
  }
}

void HttpTransfer::interrupt()
{
  qDebug() << "Interrupt file transfer: " << m_srcPath;
  m_interrupted = true;
}
