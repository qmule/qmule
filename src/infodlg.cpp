#include "infodlg.h"
#include <QDebug>

is_info_dlg::is_info_dlg(QWidget *parent) : QDialog(parent, Qt::CustomizeWindowHint |
                                                    Qt::WindowTitleHint), m_first_call(true)
{
    setupUi(this);
    m_alert.reset(new QTimer);
    m_query.reset(new QNetworkAccessManager);
    m_answer.reset(new QNetworkAccessManager);

    connect(m_alert.data(), SIGNAL(timeout()), SLOT(onTimeout()));
    connect(m_query.data(), SIGNAL(finished(QNetworkReply*)),SLOT(replyFinished(QNetworkReply*)));
    connect(pushButton, SIGNAL(clicked()), SLOT(onAccepted()));
}

void is_info_dlg::start()
{
    // start checker from one second after start was executed
    m_alert->start(1000);
}

is_info_dlg::~is_info_dlg()
{
}

void is_info_dlg::onAccepted()
{
    m_answer->get(QNetworkRequest(QUrl("http://pis.is74.ru/message_emule.php?source=emule&submit=1")));
    m_alert->start(3600000);    // one hour
}

void is_info_dlg::onTimeout()
{
    m_alert->stop();

    QString strQuery("http://pis.is74.ru/message_emule.php?source=emule");

    if (m_first_call)
    {
        strQuery += "&firstrun=1";
    }

    m_query->get(QNetworkRequest(QUrl(strQuery)));
}

void is_info_dlg::replyFinished(QNetworkReply* pReply)
{
    if (pReply->error() == QNetworkReply::NoError)
    {
        qDebug() << "reply correct";
        m_first_call = false;
        QByteArray data = pReply->readAll();
        QString strAnswer = QString::fromUtf16((const ushort*)(data.constData()), data.length());
        QRegExp re1("<body>[\n\t\s\r]*</body>", Qt::CaseInsensitive);

        // answer is not empty and html body is not empty
        if (!strAnswer.isEmpty() && !strAnswer.contains(re1))
        {
            webInfo->setHtml(strAnswer);
            show();
            return;
        }
    }

    qDebug() << "reset timer without show dialog";
    m_alert->start(3600000);
}
