#ifndef MESSAGES_WIDGET_H
#define MESSAGES_WIDGET_H

#include <QWidget>
#include "ui_messages_widget.h"

#include "qtlibed2k/qed2ksession.h"

QT_BEGIN_NAMESPACE
class QStandardItemModel;
QT_END_NAMESPACE


struct USER
{
    QString                 strName;
    libed2k::net_identifier netPoint;
};

class messages_widget : public QWidget, public Ui::messages_widget
{
    Q_OBJECT

private:
    std::vector<USER> users;
    std::vector<USER> friends;

    QStandardItemModel* model;

    QMenu* userMenu;
    QAction* userSendMessage;
    QAction* userDetails;
    QAction* userAdd;
    QAction* userBrowseFiles;
    QAction* userDelete;

public:
    messages_widget(QWidget *parent = 0);
    ~messages_widget();

public slots:
    void startChat(const QString& user_name, const libed2k::net_identifier& np);

private slots:
    void pushMessage();
    void closeTab(int index);
    void newMessage(const libed2k::net_identifier& np, const QString& hash, const QString& strMessage);
    void peerCaptchaRequest(const libed2k::net_identifier& np, const QString& hash, const QPixmap& pm);
    void peerCaptchaResult(const libed2k::net_identifier& np, const QString& hash, quint8 nResult);
    void displayListMenu(const QPoint& pos);
    void addFriend();
    void sendMessage();
};

#endif // MESSAGES_WIDGET_H
