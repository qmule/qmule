#include <QFileInfo>
#include <QDebug>

#include "wgetter.h"

wgetter::wgetter(const QString& url, const QString& dst) : m_dst(dst), m_filesystem_error(QFile::NoError)
{
    // open target file and start downloading
    m_file.reset(new QFile(dst + QString("_bak")));

    if (m_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        m_get_reply = m_nm.get(QNetworkRequest(url));
        connect(m_get_reply, SIGNAL(readyRead()), SLOT(on_data_ready()));
        connect(m_get_reply, SIGNAL(finished()), SLOT(on_data_finished()));
    }
    else
    {
        qDebug() << "wgetter: unable to create file " << m_file->errorString();
    }
}

void wgetter::on_data_ready()
{
    if (m_file->write(m_get_reply->readAll()) == -1)
    {
        m_filesystem_error = m_file->error();
        qDebug() << "unable to write " << m_file->fileName();
        m_get_reply->close();
    }
}

void wgetter::on_data_finished()
{    
    on_data_ready();
    m_file->flush();

    // check errors
    if (m_get_reply->error() || (m_filesystem_error != QFile::NoError))
    {        
        qDebug() << "error on finished " << m_get_reply->error() <<
                    " or filesystem error " << m_filesystem_error;
        // clear resources
        m_file->remove();
    }
    else
    {
        // ok, file was downloaded succesfully
        QFileInfo fi(m_dst);

        if (fi.exists())
        {
            QFile::remove(m_dst);
        }

        if (!m_file->rename(m_dst))
        {
            qDebug() << "Unable to rename file: " << m_file->error();
        }
    }

    //remove reply
    m_get_reply->deleteLater();
    m_get_reply = NULL;
}

