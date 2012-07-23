#include "status_bar.h"

#include <QStatusBar>

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

    reset();
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

void status_bar::setServerInfo(unsigned long nFiles, unsigned long nClients)
{
    QString strClients;
    QString strFiles;

    strClients.setNum(nClients);
    strFiles.setNum(nFiles);

    QString text = tr("Clients: ") + strClients + tr("|Files: ") + strFiles;
    labelInfo->setText(text);
    labelInfo->setToolTip(text);
}

void status_bar::setStatusMsg(QString strMsg)
{
    labelServer->setText(strMsg);
    labelServer->setToolTip(strMsg);
}

void status_bar::reset()
{
    setConnected(false);
    setUpDown(0, 0);
    setServerInfo(0, 0);
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