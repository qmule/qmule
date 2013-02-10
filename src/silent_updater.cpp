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
    m_build(build),
    m_reply(NULL),
    m_update_reply(NULL),
    m_filesystem_error(QFile::NoError)
{
    m_check_tm.reset(new QTimer);    
    m_nm.reset(new QNetworkAccessManager);
    connect(m_check_tm.data(), SIGNAL(timeout()), SLOT(on_check_updates()));    

    if (QFile::exists(old_filename())) QFile::remove(old_filename());
    if (QFile::exists(update_filename())) QFile::remove(update_filename());
}

silent_updater::~silent_updater()
{
    if (m_reply)
    {
        m_reply->abort();
    }

    if (m_update_reply)
    {
        m_update_reply->abort();
    }

    m_check_tm->stop();
}

void silent_updater::start()
{
    if (m_started) return;
    m_started = true;
    m_check_tm->start(10000);   // start first check after 10 seconds from program was started
}

void silent_updater::on_check_updates()
{
    // start network request
    m_check_tm->stop();
    m_update_reply = m_nm->get(QNetworkRequest(QUrl("http://tcs.is74.ru/update_qmule.php")));
    connect(m_update_reply, SIGNAL(finished()), SLOT(on_update_check_finished()));
}

void silent_updater::on_update_check_finished()
{
    // network request completed    
    if (m_update_reply->error() == QNetworkReply::NoError)
    {
        // ok, no errors
        QByteArray data = m_update_reply->readAll();
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
                if (!element.isNull() && (element.tagName() == "qmule") && !element.attribute("url").isEmpty())
                {
                    qDebug() << "url: " << element.attribute("url");

                    QRegExp vrx("(\\d+.\\d+.\\d+.\\d+)");

                    if (vrx.exactMatch(element.attribute("version")))
                    {

                        QStringList vlist = element.attribute("version").split(".");

                        if (vlist.size() == 4)
                        {
                            if (vlist.at(0).toInt() > m_major)
                            {
                                // current version is obsolete
                                emit current_version_is_obsolete(vlist.at(0).toInt(),
                                                                 vlist.at(1).toInt(),
                                                                 vlist.at(2).toInt(),
                                                                 vlist.at(3).toInt());
                            }
                            else
                            {

                                bool update = false;
                                if (vlist.at(3).toInt() > m_build)  update = true;
                                if (vlist.at(2).toInt() > m_update) update = true;
                                if (vlist.at(2).toInt() < m_update) update = false;
                                if (vlist.at(1).toInt() > m_minor)  update = true;
                                if (vlist.at(1).toInt() < m_minor)  update = false;

                                if (update)
                                {
                                    // need update !
                                    qDebug() << "new version was found " << element.attribute("version");

                                    m_file.reset(new QFile(update_filename()));
                                    if (QFile::exists(update_filename())) QFile::remove(update_filename());

                                    // check file
                                    if (!m_file->open(QIODevice::WriteOnly))
                                    {
                                        qDebug() << "can't create file " << update_filename();
                                        m_file.reset();
                                    }
                                    else
                                    {
                                        qDebug() << "start network request";
                                        // store new version
                                        m_major = vlist.at(0).toInt();
                                        m_minor = vlist.at(1).toInt();
                                        m_update= vlist.at(2).toInt();
                                        m_build = vlist.at(3).toInt();
                                        // start async request and return
                                        m_reply = m_nm->get(QNetworkRequest(QUrl(element.attribute("url"))));
                                        connect(m_reply, SIGNAL(readyRead()), SLOT(on_data_ready()));
                                        connect(m_reply, SIGNAL(finished()), SLOT(on_data_finished()));
                                        m_update_reply->deleteLater();
                                        m_update_reply = NULL;
                                        return;
                                    }
                                }
                            }
                        }
                        else
                        {
                            qDebug() << "what is it? list size incorrect - it is impossible";
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
    else
    {
        qDebug() << "error on update request " << m_update_reply->error();
    }

    m_update_reply->deleteLater();
    m_update_reply = NULL;
    // start new cycle - possible we got temporary fail
    m_check_tm->start(update_timeout);
}

void silent_updater::on_data_ready()
{
    qDebug() << "receive data size: " << m_reply->size();

    if (m_file)
    {
        if (m_file->write(m_reply->readAll()) == -1)
        {
            // we got error on write file data, possibly out of space
            qDebug() << "error on write data in update file";
            m_filesystem_error = m_file->error();
            m_reply->close();
        }
    }
}

void silent_updater::on_data_finished()
{
    qDebug() << "data finished";
    bool bCompleted = true;
    // read last part
    on_data_ready();
    m_file->flush();

    // check errors
    if (m_reply->error() || (m_filesystem_error != QFile::NoError))
    {
        bCompleted = false;
        qDebug() << "error on finished " << m_reply->error() <<
                    " or filesystem error " << m_filesystem_error;
        // clear resources
        m_file->remove();
        m_filesystem_error = QFile::NoError;
    }

    //remove updater and file holder
    m_reply->deleteLater();
    m_reply = NULL;    
    m_file.reset();

    if (bCompleted)
    {
        qDebug() << "new file download was completed - upgrade it";
        QFile f(update_filename());

#ifdef Q_WS_WIN
        if (!MoveFileExA(current_filename().toLocal8Bit(), old_filename().toLocal8Bit(), MOVEFILE_COPY_ALLOWED))
        {
            qDebug() << "error on move file : " << current_filename() << " to " << old_filename()
                     << GetLastError();
        }
#else
        if (!QFile::remove(current_filename()))
        {
            qDebug() << "unable to remove file " << current_filename();
        }
#endif
        else
        {
            qDebug() << "move completed succefully";

            if (f.copy(current_filename()))
            {
                qDebug() << "new program succesfully copied";
                QFile::setPermissions(current_filename(),
                                      QFile::permissions(current_filename()) | QFile::ExeUser | QFile::ExeGroup);
                emit new_version_ready(m_major, m_minor, m_update, m_build);
            }
            else
            {
                qDebug() << "unable to replace current file";
            }
        }
    }
    else
    {
        // start new upgrade iteration when changes weren't made
        m_check_tm->start(update_timeout);
    }
}

