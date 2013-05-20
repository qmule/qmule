
#include "httptransfer.h"
#include "misc.h"
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
        buf += "HTTP/1.1 200 OK\r\n";
        buf += QString("Content-Type: %1\r\n").arg(contentType()).toUtf8();
        buf += QString("Content-Length: %1\r\n\r\n").arg(srcFile.size()).toUtf8();

        if (m_dst->write(buf) != -1 && m_dst->waitForBytesWritten())
        {
            while(!m_interrupted)
            {
                buf = srcFile.read(256 * 1024);
                if (buf.size() == 0) break;
                if (m_dst->write(buf) == -1 || !m_dst->waitForBytesWritten())
                {
                    qDebug() << "Error: cannot send file: " << m_srcPath;
                    break;
                }
            }
        }
        else
            qDebug() << "Error: cannot send header: " << m_srcPath;

        qDebug() << "File upload completed: " << m_srcPath;
        srcFile.close();
    }
    else
        qDebug() << "Error: cannot open file: " << m_srcPath;

    m_dst->disconnectFromHost();
}

void HttpTransfer::interrupt()
{
    qDebug() << "Interrupt file transfer: " << m_srcPath;
    m_interrupted = true;
}

QString HttpTransfer::contentType()
{
    QString ext = misc::file_extension(m_srcPath).toUpper();
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
