#ifndef STATUS_WIDGET_H
#define STATUS_WIDGET_H

#include <QWidget>
#include "ui_status_widget.h"

class status_widget : public QWidget, private Ui::status_widget
{
    Q_OBJECT

    QString m_strServer;
    unsigned int m_nClientId;
    int m_nFiles;
    int m_nUsers;

public:
    status_widget(QWidget *parent = 0);
    ~status_widget();
    void addLogMessage(QString log_message);
    void setDisconnectedInfo();
    void updateConnectedInfo();
    void serverAddress(QString strServer);
    void serverInfo(QString strInfo);
    void serverStatus(int nFiles, int nUsers);
    void clientID(unsigned int nClientId);
public slots:
    void addHtmlLogMessage(const QString& msg);
};

#endif // STATUS_WIDGET_H
