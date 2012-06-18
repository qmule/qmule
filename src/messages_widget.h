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
    int lastMessageTab;

    QIcon imgMsg1;
    QIcon imgMsg2;

public:
    messages_widget(QWidget *parent = 0);
    ~messages_widget();
    void setNewMessageImg(int state);

protected:
    //virtual void showEvent(QShowEvent* e);
    //virtual void focusInEvent(QFocusEvent* e);
    virtual bool event(QEvent* e);

public slots:
    void startChat(const QString& user_name, const libed2k::net_identifier& np);

private slots:
    void pushMessage();
    void closeCurrentTab();
    void closeTab(int index);
    void newMessage(const libed2k::net_identifier& np, const QString& hash, const QString& strMessage);
    void peerCaptchaRequest(const libed2k::net_identifier& np, const QString& hash, const QPixmap& pm);
    void peerCaptchaResult(const libed2k::net_identifier& np, const QString& hash, quint8 nResult);
    void displayListMenu(const QPoint& pos);
    void addFriend();
    void deleteFriend();
    void sendMessage();
    void selectTab(int nTabNum);

signals:
    void newMessage();
    void stopMessageNotification();
};

#endif // MESSAGES_WIDGET_H
