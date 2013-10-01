#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <QWidget>
#include "ui_status_bar.h"

QT_BEGIN_NAMESPACE
class QStatusBar;
QT_END_NAMESPACE

class status_bar : public QWidget, public Ui::status_bar
{
    Q_OBJECT
    QStatusBar* m_bar;
    QIcon imgConnected;
    QIcon imgDisconnected;
    QIcon Up0Down0;
    QIcon Up1Down0;
    QIcon Up0Down1;
    QIcon Up1Down1;
    QIcon imgMsg1;
    QIcon imgMsg2;
    QIcon imgEmpty;

    struct server_info {
        unsigned long m_nClients;
        unsigned long m_nFiles;

        server_info(unsigned long nClients = 0, unsigned long nFiles = 0):
            m_nClients(nClients), m_nFiles(nFiles){}

        server_info operator+(server_info si){
            return server_info(m_nClients + si.m_nClients, m_nFiles + si.m_nFiles);
        }
    };

    QMap<QString, server_info> m_servers;

public:
    status_bar(QWidget *parent, QStatusBar *bar);
    ~status_bar();
    void setConnected(bool conn);
    void setUpDown(unsigned long nUp, unsigned long nDown);
    void setServerInfo(const QString& sid, unsigned long nFiles, unsigned long nClients);
    void serverInfoChanged();
    void setStatusMsg(QString strMsg);
    void setNewMessageImg(int state);
    void reset(const QString& sid);

private slots:
    void doubleClickNewMsg();

signals:
    void stopMessageNotification();
};

#endif // STATUS_BAR_H
