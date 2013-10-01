
#include <numeric>
#include <QStatusBar>

#include "status_bar.h"
#include "misc.h"

status_bar::status_bar(QWidget *parent, QStatusBar *bar)
    : QWidget(parent) , m_bar(bar)
{
    setupUi(this);

    m_bar->addPermanentWidget(this, 1);
    m_bar->setContentsMargins(-1, -1, -1, -1);
    m_bar->setFixedHeight(21);

    imgConnected.addFile(QString::fromUtf8(":/emule/statusbar/connected.ico"), QSize(), QIcon::Normal, QIcon::Off);
    imgDisconnected.addFile(QString::fromUtf8(":/emule/statusbar/disconnected.ico"), QSize(), QIcon::Normal, QIcon::Off);

    Up0Down0.addFile(QString::fromUtf8(":/emule/statusbar/Up0down0.ico"), QSize(), QIcon::Normal, QIcon::Off);
    Up0Down1.addFile(QString::fromUtf8(":/emule/statusbar/Up0down1.ico"), QSize(), QIcon::Normal, QIcon::Off);
    Up1Down0.addFile(QString::fromUtf8(":/emule/statusbar/Up1down0.ico"), QSize(), QIcon::Normal, QIcon::Off);
    Up1Down1.addFile(QString::fromUtf8(":/emule/statusbar/Up1down1.ico"), QSize(), QIcon::Normal, QIcon::Off);

    imgMsg1.addFile(QString::fromUtf8(":/emule/statusbar/Message.ico"), QSize(), QIcon::Normal, QIcon::Off);
    imgMsg2.addFile(QString::fromUtf8(":/emule/statusbar/MessagePending.ico"), QSize(), QIcon::Normal, QIcon::Off);
    imgEmpty.addFile(QString::fromUtf8(":/emule/statusbar/empty.ico"), QSize(), QIcon::Normal, QIcon::Off);

    labelInfoImg->setPixmap(QIcon(":/emule/common/User.ico").pixmap(16,16));

    connect(labelMsg, SIGNAL(doubleClicked()), this, SLOT(doubleClickNewMsg()));

    reset(QString());
}

status_bar::~status_bar()
{
}

void status_bar::setConnected(bool conn)
{
    if (conn)
    {
        labelNetImg->setPixmap(imgConnected.pixmap(16, 16));
        labelNet->setText(tr("eD2K:Connected"));
        labelNet->setToolTip(tr("eD2K:Connected"));
    }
    else
    {
        labelNetImg->setPixmap(imgDisconnected.pixmap(16, 16));
        labelNet->setText(tr("eD2K:Disconnected"));
        labelNet->setToolTip(tr("eD2K:Disconnected"));
    }
}

void status_bar::setUpDown(unsigned long nUp, unsigned long nDown)
{
    QIcon icon = nUp > 0 ? (nDown > 0 ? Up1Down1 : Up1Down0) : (nDown > 0 ? Up0Down1 : Up0Down0);
    labelSpeedImg->setPixmap(icon.pixmap(16, 16));
    QString text = tr("Upload:") + QString(" %1/%2, ").arg(misc::friendlyUnit(nUp)).arg(tr("s")) +
                   tr("Download:") + QString(" %1/%2").arg(misc::friendlyUnit(nDown)).arg(tr("s"));
    labelSpeed->setText(text);
    labelSpeed->setToolTip(text);
}

void status_bar::setServerInfo(const QString& sid, unsigned long nFiles, unsigned long nClients)
{
    m_servers[sid].m_nFiles += nFiles;
    m_servers[sid].m_nClients += nClients;

    serverInfoChanged();
}

void status_bar::serverInfoChanged()
{
    QList<server_info> infos = m_servers.values();
    server_info suminfo = std::accumulate(infos.begin(), infos.end(), server_info());

    QString strClients;
    QString strFiles;

    strClients.setNum(suminfo.m_nClients);
    strFiles.setNum(suminfo.m_nFiles);

    QString text = tr("Clients: ") + strClients + tr("|Files: ") + strFiles;
    labelInfo->setText(text);
    labelInfo->setToolTip(text);
}

void status_bar::setStatusMsg(QString strMsg)
{
    labelServer->setText(strMsg);
    labelServer->setToolTip(strMsg);
}

void status_bar::reset(const QString& sid)
{
    setConnected(!m_servers.empty());
    setUpDown(0, 0);

    m_servers.remove(sid);
    serverInfoChanged();
}

void status_bar::setNewMessageImg(int state)
{
    switch (state)
    {
        case 0:
        {
            labelMsg->setPixmap(imgEmpty.pixmap(16, 16));
            break;
        }
        case 1:
        {
            labelMsg->setPixmap(imgMsg1.pixmap(16, 16));
            break;
        }
        case 2:
        {
            labelMsg->setPixmap(imgMsg2.pixmap(16, 16));
            break;
        }
    }
}

void status_bar::doubleClickNewMsg()
{
    emit stopMessageNotification();
}
