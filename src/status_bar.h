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

public:
    status_bar(QWidget *parent, QStatusBar *bar);
    ~status_bar();
    void setConnected(bool conn);
    void setUpDown(unsigned long nUp, unsigned long nDown);
    void setServerInfo(unsigned long nFiles, unsigned long nClients);
    void setStatusMsg(QString strMsg);
    void setNewMessageImg(int state);
    void reset();
};

#endif // STATUS_BAR_H
