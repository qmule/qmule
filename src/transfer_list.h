#ifndef TRANSFER_LIST_H
#define TRANSFER_LIST_H

#include <QWidget>
#include <QMainWindow>
#include "libed2k/packet_struct.hpp"

class MainWindow;
class TransferListWidget;
class PeerListWidget;


QT_BEGIN_NAMESPACE
class QString;
class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QPushButton;
class QSpacerItem;
class QToolBar;
QT_END_NAMESPACE

class transfer_list : public  QMainWindow
{
    Q_OBJECT

private:
    static const int topRowBtnCnt = 4;
    static const int bottomRowBtnCnt = 2;
    
    QStringList btnText;

    QVBoxLayout* vboxLayout;
    QSplitter* hSplitter;
    QWidget* verticalLayoutWidget1;
    QWidget* verticalLayoutWidget2;
    QVBoxLayout* vboxLayout1;
    QVBoxLayout* vboxLayout2;
    TransferListWidget* transferList;
    PeerListWidget* peersList;
    
    QHBoxLayout* hboxLayout1;
    QHBoxLayout* hboxLayout2;
    QPushButton* btnSwitch;
    QPushButton* btnSwitch2;
    QPushButton** topRowButtons;
    QPushButton** bottomRowButtons;
    QIcon* icons;

    QSpacerItem* horizontalSpacer;
    QSpacerItem* horizontalSpacer2;

    QTimer *refreshTimer;

    QWidget* currTopWidget;
    QWidget* currBottomWidget;

    QAction* actionOpen;
    QAction* actionDelete;
    QAction* actionStart;
    QAction* actionPause;

    QToolBar* mainToolBar;

public:
    transfer_list(QWidget *parent, MainWindow *mainWindow);
    ~transfer_list();
    TransferListWidget* getTransferList() { return transferList; }

private:
    QPushButton* createFlatButton(QIcon& icon);
    void ProcessTopButton(int btn_num);
    void ProcessBottomButton(int btn_num);

private slots:
    void btnSwitchClick();
    void btnSwitchClick2();

    void btnTopClick();
    void btnBottomClick();

    void refreshPeers();
    void addPeerToFriends(const QString& user_name, const libed2k::net_identifier& np);
    void sendMessageToPeer(const QString& user_name, const libed2k::net_identifier& np);

signals:
    void addFriend(const QString& user_name, const libed2k::net_identifier& np);
    void sendMessage(const QString& user_name, const libed2k::net_identifier& np);
};

#endif // TRANSFER_LIST_H
