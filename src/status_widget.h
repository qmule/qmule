#ifndef STATUS_WIDGET_H
#define STATUS_WIDGET_H

#include <QWidget>
#include "ui_status_widget.h"

struct server_info
{
    unsigned int m_nClientId;
    int m_nFiles;
    int m_nUsers;
    QString m_strName;

    server_info(): m_nClientId(0), m_nFiles(0), m_nUsers(0){}
};

class status_widget : public QWidget, private Ui::status_widget
{
    Q_OBJECT

    QMap<QString, server_info> m_servers;

public:
    status_widget(QWidget *parent = 0);
    ~status_widget();
    void addLogMessage(QString log_message);
    void addLogMessage(const QString&, const QString&);
    void setDisconnectedInfo(const QString& sid);
    void updateConnectedInfo();
    void serverAddress(QString strServer);
    void serverInfo(QString strInfo);
    void serverIdentity(const QString&, const QString&);
    void serverStatus(const QString& sid, int nFiles, int nUsers);
    void clientID(const QString& sid, unsigned int nClientId);
public slots:
    void addHtmlLogMessage(const QString& msg);
};

#endif // STATUS_WIDGET_H
