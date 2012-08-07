#include "silent_updater.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QRegExp>
#include <QStringList>
#include <QDebug>
#include <QApplication>
#ifdef Q_WS_WIN
#include <windows.h>
#endif

QString update_filename()
{
    return QApplication::applicationFilePath() + QString(".update");
}

QString old_filename()
{
    return QApplication::applicationFilePath() + QString(".old");
}

QString current_filename()
{
    return QApplication::applicationFilePath();
}


silent_updater::silent_updater(int major, int minor, int update, int build, QObject *parent) :
    QObject(parent),
    m_download_aborted(false),
    m_started(false),
    m_major(major),
    m_minor(minor),
    m_update(update),
    m_build(build)
{
    m_check_tm.reset(new QTimer);
    m_version_nc.reset(new QNetworkAccessManager);
    m_downloader.reset(new QNetworkAccessManager);
    connect(m_check_tm.data(), SIGNAL(timeout()), SLOT(on_check_updates()));
    connect(m_version_nc.data(), SIGNAL(finished(QNetworkReply*)),SLOT(on_network_check_completed(QNetworkReply*)));

    if (QFile::exists(old_filename())) QFile::remove(old_filename());
    if (QFile::exists(update_filename())) QFile::remove(update_filename());
}

silent_updater::~silent_updater()
{
    if (m_reply)
    {
        m_reply->abort();
    }

    m_check_tm->stop();
}

void silent_updater::start()
{
    if (m_started) return;
    m_started = true;
    m_check_tm->start(1000);
}

void silent_updater::on_check_updates()
{
    // start network request
    m_check_tm->stop();
    m_version_nc->get(QNetworkRequest(QUrl("http://tcs.is74.ru/update.php")));
}

void silent_updater::on_network_check_completed(QNetworkReply *pReply)
{
    // network request completed
    if (pReply->error() == QNetworkReply::NoError)
    {
        // ok, no errors
        QByteArray data = pReply->readAll();
        QString xml = QString::fromUtf8(data.constData());

        QDomDocument doc;
        QString errorStr;
        int errorLine;
        int errorColumn;

        if (doc.setContent(xml, true, &errorStr, &errorLine, &errorColumn))
        {
            QDomElement root = doc.documentElement();
            QDomNode node = root.firstChild();

            while(!node.isNull())
            {
                QDomElement element  = node.toElement();

                // validate element
                if (!element.isNull() && (element.tagName() == "emule") && !element.attribute("url").isEmpty())
                {
                    qDebug() << "url: " << element.attribute("url");

                    QRegExp vrx("(\\d+.\\d+.\\d+.\\d+)");

                    if (vrx.exactMatch(element.attribute("version")))
                    {

                        QStringList vlist = element.attribute("version").split(".");

                        if (vlist.size() == 4)
                        {
                            if (vlist.at(0).toInt() > m_major ||
                                vlist.at(1).toInt() > m_minor ||
                                vlist.at(2).toInt() > m_update ||
                                vlist.at(3).toInt() > m_build)
                            {
                                // need update !
                                qDebug() << "new version was found " << element.attribute("version");

                                m_file.reset(new QFile(update_filename()));
                                if (QFile::exists(update_filename())) QFile::remove(update_filename());

                                // check file
                                if (!m_file->open(QIODevice::WriteOnly))
                                {
                                    m_file.reset();
                                }
                                else
                                {
                                    // start async request and return
                                    m_reply = m_downloader->get(QNetworkRequest(QUrl(element.attribute("url"))));
                                    connect(m_reply, SIGNAL(readyRead()), SLOT(on_data_ready()));
                                    connect(m_reply, SIGNAL(finished()), SLOT(on_data_finished()));
                                    return;
                                }
                            }
                        }
                        else
                        {
                            qDebug() << "what is it? list size incorrect - it is impossible"
                        }
                    }
                    else
                    {
                        qDebug() << "unrecognized version: " << element.attribute("version");
                    }
                }

                node = node.nextSibling();
            }
        }
        else
        {
            qDebug() << "fail on decode xml " << xml << " with error " << errorStr;
        }
    }

    // start new cycle - possible we got temporary fail
    m_check_tm->start(10000);
}

void silent_updater::on_data_ready()
{
    if (m_file)
    {
        m_file->write(m_reply->readAll());
    }
}

void silent_updater::on_data_finished()
{
    qDebug() << "data finished";
    bool bCompleted = true;
    // read last part
    on_data_ready();
    m_file->flush();

    if (m_reply->error())
    {
        bCompleted = false;
        qDebug() << m_reply->error();
        // error on download
        m_file->remove();
    }

    //remove updater and file holder
    m_reply->deleteLater();
    m_reply = NULL;
    m_file.reset();

    if (bCompleted)
    {
        qDebug() << "new file download was completed - upgrade it";
#ifdef Q_WS_WIN
        if (!MoveFileExA(current_filename().toLocal8Bit(), old_filename().toLocal8Bit(), MOVEFILE_COPY_ALLOWED))
        {
            qDebug() << "error on move file : " << current_filename() << " to " << old_filename()
                     << GetLastError();
        }
        else
        {
            qDebug() << "move completed succefully";
            QFile f(update_filename());

            if (f.copy(current_filename()))
            {
                qDebug() << "new program succesfully copied";
            }
            else
            {
                qDebug() << "unable to replace current file";
            }
        }
#endif
    }
    else
    {
        // start new upgrade iteration when changes weren't made
        m_check_tm->start(10000);
    }
}

