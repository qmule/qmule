#ifndef MESSAGES_WIDGET_H
#define MESSAGES_WIDGET_H

#include <QWidget>
#include "ui_messages_widget.h"
#include "preferences.h"

#include "qtlibed2k/qed2ksession.h"

QT_BEGIN_NAMESPACE
class QStandardItemModel;
QT_END_NAMESPACE


struct USER
{
    QString                 strName;
    libed2k::net_identifier netPoint;
    int                     connected;
    QTextEdit*              edit;
    int                     nTabNum;
    QString                 post_msg;

    USER();
    USER(const Preferences& pref);
    void save(Preferences& pref) const;
};

class messages_widget : public QWidget, public Ui::messages_widget
{
    Q_OBJECT

private:
    std::vector<USER> users;
    std::vector<USER> friends;
    QList<libed2k::net_identifier> connectedPeers;

    QStandardItemModel* model;

    QMenu* userMenu;
    QAction* userSendMessage;
    QAction* userDetails;
    QAction* userAdd;
    QAction* userBrowseFiles;
    QAction* userDelete;

    QAction* userAddTab;
    QAction* userDetailsTab;
    QAction* userDeleteTab;
    QAction* closeCurTab;
    QMenu* tabMenu;
    QMenu* tabMenuFriend;
    int tabMenuNum;

    int lastMessageTab;

    QColor greenColor;
    QColor blueColor;
    QColor systemColor;

    QIcon imgMsg1;
    QIcon imgMsg2;

public:
    messages_widget(QWidget *parent = 0);
    ~messages_widget();
    void setNewMessageImg(int state);
    void save() const;
    void load();

protected:
    //virtual void showEvent(QShowEvent* e);
    //virtual void focusInEvent(QFocusEvent* e);
    virtual bool event(QEvent* e);
    virtual bool eventFilter(QObject *obj, QEvent *e);

private:
    void enableButtons(bool enable = true);
    void addMessage(QTextEdit* edit, QString name, QString msg, QColor& color);
    std::vector<USER>::iterator findUser(const libed2k::net_identifier& np);
    void setFriendIcon(const libed2k::net_identifier& np, bool connected);

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
    void displayTabMenu(const QPoint& pos);
    void addFriend();
    void deleteFriend();
    void sendMessage();
    void friendDetails();
    void addTabFriend();
    void deleteTabFriend();
    void closeTab();
    void userTabDetails();
    void selectTab(int nTabNum);
    void peerConnected(const libed2k::net_identifier& np, const QString&, bool bActive);
    void peerDisconnected(const libed2k::net_identifier& np, const QString&, const libed2k::error_code ec);
    void requestUserDirs();

signals:
    void newMessage();
    void stopMessageNotification();
};

#endif // MESSAGES_WIDGET_H
