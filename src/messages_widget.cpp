#include <QDateTime>
#include <QMenu>
#include <QMessageBox>
#include <QTextEdit>
#include <QKeyEvent>
#include <QStandardItemModel>

#include "libed2k/util.hpp"
#include "libed2k/session.hpp"
#include "transport/session.h"
#include "qed2kpeerhandle.h"

#include "add_friend.h"
#include "user_properties.h"
#include "messages_widget.h"

using namespace libed2k;

USER::USER() : strName(), netPoint(), connected(-1), edit(NULL), nTabNum(-1), post_msg()
{
}

USER::USER(const Preferences& pref) : connected(-1), edit(NULL), nTabNum(-1), post_msg()
{
    strName = pref.value("Username", QString()).toString();
    netPoint.m_nIP = pref.value("IP", 0).toUInt();
    netPoint.m_nPort = pref.value("Port", 0).toUInt();
}

void USER::save(Preferences& pref) const
{
    pref.setValue("Username", strName);
    pref.setValue("IP", netPoint.m_nIP);
    pref.setValue("Port", netPoint.m_nPort);
}

messages_widget::messages_widget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    lastMessageTab = -1;

    imgFriends->setPixmap(QIcon(":/emule/users/Friend.ico").pixmap(16, 16));
    imgMessages->setPixmap(QIcon(":/emule/users/Message.ico").pixmap(16, 16));
    
    label_name->setText("-");
    label_hash->setText("-");
    label_programm->setText("-");
    label_IP->setText("-");
    label_port->setText("-");

    QList<int> sizes;
    sizes.append(300);
    sizes.append(1000000);
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    userMenu = new QMenu(listFriends);
    userMenu->setObjectName(QString::fromUtf8("userMenu"));
    userMenu->setTitle(tr("Friends"));

    userSendMessage = new QAction(this);
    userSendMessage->setObjectName(QString::fromUtf8("userSendMessage"));
    userSendMessage->setText(tr("Send message"));
    userSendMessage->setIcon(QIcon(":/emule/users/UserMessage.ico"));

    userDetails = new QAction(this);
    userDetails->setObjectName(QString::fromUtf8("userDetails"));
    userDetails->setText(tr("Details..."));
    userDetails->setIcon(QIcon(":/emule/users/UserDetails.ico"));

    userAdd = new QAction(this);
    userAdd->setObjectName(QString::fromUtf8("userAdd"));
    userAdd->setText(tr("Add..."));
    userAdd->setIcon(QIcon(":/emule/users/UserAdd.ico"));

    userBrowseFiles = new QAction(this);
    userBrowseFiles->setObjectName(QString::fromUtf8("userBrowseFiles"));
    userBrowseFiles->setText(tr("Browse files"));
    userBrowseFiles->setIcon(QIcon(":/emule/users/UserFiles.ico"));

    userDelete = new QAction(this);
    userDelete->setObjectName(QString::fromUtf8("userDelete"));
    userDelete->setText(tr("Delete"));
    userDelete->setIcon(QIcon(":/emule/users/UserDelete.ico"));

    userMenu->addAction(userSendMessage);
    userMenu->addAction(userDetails);
    userMenu->addAction(userAdd);
    userMenu->addAction(userBrowseFiles);
    userMenu->addAction(userDelete);
        
    userAddTab = new QAction(this);
    userAddTab->setObjectName(QString::fromUtf8("userAddTab"));
    userAddTab->setText(tr("Add to the friend list"));
    userAddTab->setIcon(QIcon(":/emule/users/UserAdd.ico"));
      
    userDetailsTab = new QAction(this);
    userDetailsTab->setObjectName(QString::fromUtf8("userDetailsTab"));
    userDetailsTab->setText(tr("Details..."));
    userDetailsTab->setIcon(QIcon(":/emule/users/UserDetails.ico"));

    userDeleteTab = new QAction(this);
    userDeleteTab->setObjectName(QString::fromUtf8("userDeleteTab"));
    userDeleteTab->setText(tr("Delete"));
    userDeleteTab->setIcon(QIcon(":/emule/users/UserDelete.ico"));

    closeCurTab = new QAction(userAdd);
    closeCurTab = new QAction(this);
    closeCurTab->setObjectName(QString::fromUtf8("closeCurTab"));
    closeCurTab->setText(tr("Close"));

    tabMenu = new QMenu(tabWidget);
    tabMenu->setObjectName(QString::fromUtf8("tabMenu"));
    tabMenu->addAction(userDetailsTab);
    tabMenu->addAction(userAddTab);
    tabMenu->addAction(closeCurTab);
        
    tabMenuFriend = new QMenu(tabWidget);
    tabMenuFriend->setObjectName(QString::fromUtf8("tabMenuFriend"));
    tabMenuFriend->addAction(userDetailsTab);
    tabMenuFriend->addAction(userDeleteTab);
    tabMenuFriend->addAction(closeCurTab);

    model = new QStandardItemModel(0, 1);
    listFriends->setModel(model);
    listFriends->setContextMenuPolicy(Qt::CustomContextMenu);

    greenColor = "#00C800";
    blueColor = "#00AFFF";
    systemColor = "#008000";

    connect(listFriends, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayListMenu(const QPoint&)));
    connect(tabWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayTabMenu(const QPoint&)));
    
    connect(userAdd, SIGNAL(triggered()), this, SLOT(addFriendDlg()));
    connect(userSendMessage, SIGNAL(triggered()), this, SLOT(sendMessage()));
    connect(userDelete, SIGNAL(triggered()), this, SLOT(deleteFriend()));    
    connect(userBrowseFiles, SIGNAL(triggered()), this, SLOT(requestUserDirs()));
    connect(userDetails, SIGNAL(triggered()), this, SLOT(friendDetails()));

    connect(userAddTab, SIGNAL(triggered()), this, SLOT(addTabFriend()));
    connect(userDeleteTab, SIGNAL(triggered()), this, SLOT(deleteTabFriend()));
    connect(closeCurTab, SIGNAL(triggered()), this, SLOT(closeTab()));
    connect(userDetailsTab, SIGNAL(triggered()), this, SLOT(userTabDetails()));

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(tabWidget, SIGNAL(currentChanged (int)), this, SLOT(selectTab(int)));
    connect(btnSend, SIGNAL(clicked()), this, SLOT(pushMessage()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(closeCurrentTab()));
    connect(Session::instance()->get_ed2k_session(), 
            SIGNAL(peerMessage(const libed2k::net_identifier&, const QString&, const QString&)), 
            this, SLOT(newMessage(const libed2k::net_identifier&, const QString&, const QString&)));
    connect(Session::instance()->get_ed2k_session(), 
            SIGNAL(peerCaptchaRequest(const libed2k::net_identifier&, const QString&, const QPixmap&)), 
            this, SLOT(peerCaptchaRequest(const libed2k::net_identifier&, const QString&, const QPixmap&)));
    connect(Session::instance()->get_ed2k_session(), 
            SIGNAL(peerCaptchaResult(const libed2k::net_identifier&, const QString&, quint8)), 
            this, SLOT(peerCaptchaResult(const libed2k::net_identifier&, const QString&, quint8)));

    connect(Session::instance()->get_ed2k_session(),
    		SIGNAL(peerConnected(const libed2k::net_identifier&, const QString&, bool)),
            this, SLOT(peerConnected(const libed2k::net_identifier&, const QString&, bool)));
    connect(Session::instance()->get_ed2k_session(),
            SIGNAL(peerDisconnected(const libed2k::net_identifier&, const QString&, const libed2k::error_code)),
            this, SLOT(peerDisconnected(const libed2k::net_identifier&, const QString&, const libed2k::error_code)));

    connect(listFriends->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(friendSelected(const QModelIndex&, const QModelIndex&)));

    imgMsg1.addFile(QString::fromUtf8(":/emule/statusbar/Message.ico"), QSize(), QIcon::Normal, QIcon::Off);
    imgMsg2.addFile(QString::fromUtf8(":/emule/statusbar/MessagePending.ico"), QSize(), QIcon::Normal, QIcon::Off);

    textMsg->installEventFilter(this);

    enableButtons(false);

    setFocusPolicy(Qt::StrongFocus);
    load();
}

messages_widget::~messages_widget()
{
    save();
    lastMessageTab = -1;
    int nSize = tabWidget->count();
    for (int ii = 0; ii < nSize; ii++)
    {
        QTextEdit* edit = qobject_cast<QTextEdit*>(tabWidget->widget(0));
        tabWidget->removeTab(0);
        delete edit;
    }
}

void messages_widget::closeTab(int index)
{
    if (users.size() > index)
        users.erase(users.begin() + index);

    QTextEdit* edit = qobject_cast<QTextEdit*>(tabWidget->widget(index));
    tabWidget->removeTab(index);
    delete edit;

    if (!tabWidget->count())
        enableButtons(false);
}

void messages_widget::startChat(const QString& user_name, const libed2k::net_identifier& np)
{
    QTextEdit* edit = new QTextEdit(this);
    edit->setReadOnly(true);
    QTextOption to;
    to.setTextDirection(Qt::LeftToRight);
    edit->document()->setDefaultTextOption(to);
    int new_tab = tabWidget->addTab(edit, QIcon(":/emule/users/Chat.ico"), user_name);    
    tabWidget->setCurrentIndex(new_tab);

    USER user;
    user.strName = user_name;
    user.netPoint = np;
    user.edit = edit;
    user.nTabNum = new_tab;
    if (connectedPeers.contains(np))
        user.connected = true;
    users.push_back(user);

    addSystemMessage(edit, tr("*** Begin chat: ") + user_name);

    enableButtons();
    textMsg->setFocus();
}

void messages_widget::pushMessage()
{
    QString msg = textMsg->toPlainText();
    if (tabWidget->currentIndex() < 0 || !msg.length())
        return;

    USER& user = users[tabWidget->currentIndex()];
    if (user.post_msg.length())
        return;
    
    textMsg->clear();

    QTextEdit* edit = qobject_cast<QTextEdit*>(tabWidget->widget(tabWidget->currentIndex()));

    if (user.connected > 0)
    {
        Preferences pref;
        addMessage(edit, pref.nick(), msg, greenColor);
    }
    else
    {
        user.post_msg = msg;
        user.connected = -1;

        addSystemMessage(edit, tr("*** Connecting... "));
    }

    QED2KPeerHandle::getPeerHandle(user.netPoint).sendMessageToPeer(msg);
    textMsg->setFocus();
}

void messages_widget::newMessage(const libed2k::net_identifier& np, const QString& hash, const QString& strMessage)
{
    std::vector<USER>::iterator it = findUser(np);
    if ( it != users.end())
    {
        addMessage(it->edit, it->strName, strMessage, blueColor);

        lastMessageTab = it->nTabNum;

        if (!this->isActiveWindow() || !this->isVisible() || it->nTabNum != tabWidget->currentIndex())
        {
            tabWidget->setTabFontColor(it->nTabNum, QColor(Qt::darkRed));
            emit newMessage();
        }
    }
    else
    {
        QString name = QED2KPeerHandle::getPeerHandle(np).getUserName();
        startChat(name, np);

        int nTab = tabWidget->currentIndex();
        QTextEdit* edit = qobject_cast<QTextEdit*>(tabWidget->widget(nTab));

        addMessage(edit, name, strMessage, blueColor);

        lastMessageTab = nTab;
        if (!this->isActiveWindow() || !this->isVisible())
        {
            tabWidget->setTabFontColor(nTab, QColor(Qt::darkRed));
            emit newMessage();
        }
    }
}

void messages_widget::peerCaptchaRequest(const libed2k::net_identifier& np, const QString& hash, const QPixmap& pm)
{
    std::vector<USER>::iterator it = findUser(np);
    if ( it != users.end())
    {
        addSystemMessage(it->edit, tr("*** To avoid spam user is asking for captcha authentification. Please enter symbols on the picture below:\n"));
        
        QTextCursor cursor = it->edit->textCursor();
        cursor.insertImage(pm.toImage());
        it->edit->append("\n");
    }
}

void messages_widget::peerCaptchaResult(const libed2k::net_identifier& np, const QString& hash, quint8 nResult)
{
    std::vector<USER>::iterator it = findUser(np);
    if ( it != users.end())
    {
        if (nResult)
        {
            addSystemMessage(it->edit, tr("*** Your answer is incorrect and message is ignored. You may request captcha again by sending new message."));
        }
        else
        {
            addSystemMessage(it->edit, tr("*** Your answer is correct. User has recived your message."));
        }
    }
}

void messages_widget::displayListMenu(const QPoint& pos) 
{
    QModelIndex index = listFriends->currentIndex();

    if (!index.isValid())
    {
        userSendMessage->setEnabled(false);
        userDetails->setEnabled(false);
        userBrowseFiles->setEnabled(false);
        userDelete->setEnabled(false);
    }
    else
    {
        userSendMessage->setEnabled(true);
        userDetails->setEnabled(true);
        userBrowseFiles->setEnabled(true);
        userDelete->setEnabled(true);
    }

    userMenu->exec(QCursor::pos());
}

void messages_widget::displayTabMenu(const QPoint& pos) 
{
    tabMenuNum = tabWidget->getTabNum(pos);
    if (tabMenuNum < 0)
        return;

    USER user = users[tabMenuNum];
    std::vector<USER>::iterator it;
    for (it = friends.begin(); it != friends.end(); ++it)
        if (it->netPoint == user.netPoint)
            break;

    if (it != friends.end())
        tabMenuFriend->exec(QCursor::pos());
    else
        tabMenu->exec(QCursor::pos());
}

void messages_widget::addFriendDlg()
{
    add_friend dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString user_name = dlg.getName();
        libed2k::net_identifier netPoint;
        netPoint.m_nIP = address2int(ip::address::from_string(dlg.getIP().toUtf8().constData()));
        netPoint.m_nPort = dlg.getPort().toInt();
        addFriend(user_name, netPoint);
    }
}

void messages_widget::addFriend(const QString& user_name, const libed2k::net_identifier& np)
{
    USER new_friend;
    new_friend.strName = user_name;
    if (!new_friend.strName.length())
        new_friend.strName = np.m_nIP;
    new_friend.netPoint = np;

    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 0), new_friend.strName);

    if (connectedPeers.contains(new_friend.netPoint))
        model->item(row)->setIcon(QIcon(":/emule/users/Friends3.ico"));
    else
        model->item(row)->setIcon(QIcon(":/emule/users/Friends1.ico"));

    friends.push_back(new_friend);
}

void messages_widget::deleteFriend()
{
    if (QMessageBox::question(0, "qMule", tr("Do you realy want to delete a friend?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) 
        return;

    QModelIndex index = listFriends->currentIndex();

    if (!index.isValid())
        return;

    int num = index.row();

    model->removeRow(num);
    friends.erase(friends.begin() + num);    
}

void messages_widget::sendMessage()
{
    QModelIndex index = listFriends->currentIndex();

    if (!index.isValid())
        return;

    int num = index.row();

    std::vector<USER>::iterator it;
    int tab_num = 0;
    for (it = users.begin(); it != users.end();++it)
    {
        if (friends[num].netPoint == it->netPoint)
            break;
        tab_num++;
    }

    if (it != users.end())
    {
        tabWidget->setCurrentIndex(tab_num);
    }
    else
    {
        startChat(friends[num].strName, friends[num].netPoint);
    }
}

void messages_widget::friendDetails()
{
    QModelIndex index = listFriends->currentIndex();

    if (!index.isValid())
        return;

    int num = index.row();

    user_properties dlg(this, friends[num].strName, friends[num].netPoint);
    dlg.exec();
}

void messages_widget::addTabFriend()
{
    USER user = users[tabMenuNum];

    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 0), user.strName);

    if (connectedPeers.contains(user.netPoint))
        model->item(row)->setIcon(QIcon(":/emule/users/Friends3.ico"));
    else
        model->item(row)->setIcon(QIcon(":/emule/users/Friends1.ico"));

    friends.push_back(user);
}

void messages_widget::deleteTabFriend()
{
    if (QMessageBox::question(0, "qMule", tr("Do you realy want to delete a friend?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) 
        return;

    USER user = users[tabMenuNum];
    std::vector<USER>::iterator it;
    int num = 0;
    for (it = friends.begin(); it != friends.end(); ++it, ++num)
        if (it->netPoint == user.netPoint)
            break;

    if (it != friends.end())
    {
        model->removeRow(num);
        friends.erase(it);    
    }
}

void messages_widget::closeTab()
{
    closeTab(tabMenuNum);
}

void messages_widget::userTabDetails()
{
    user_properties dlg(this, users[tabMenuNum].strName, users[tabMenuNum].netPoint);
    dlg.exec();
}

void messages_widget::selectTab(int nTabNum)
{
    if (lastMessageTab == nTabNum && lastMessageTab >= 0)
    {
        emit stopMessageNotification();
        lastMessageTab = -1;
    }
}

void messages_widget::setNewMessageImg(int state)
{
    if (lastMessageTab < 0)
        return;

    switch (state)
    {
        case 0:
        {
            tabWidget->setTabIcon(lastMessageTab, QIcon(":/emule/users/Chat.ico"));
            tabWidget->setTabFontColor(lastMessageTab, QColor(Qt::black));
            break;
        }
        case 1:
        {
            tabWidget->setTabIcon(lastMessageTab, imgMsg1);
            break;
        }
        case 2:
        {
            tabWidget->setTabIcon(lastMessageTab, imgMsg2);
            break;
        }
    }
}

void messages_widget::save() const
{
    Preferences pref;
    pref.beginGroup("ED2KFriends");
    pref.beginWriteArray("Friends", friends.size());

    int i = 0;
    foreach(const USER& u, friends)
    {
        pref.setArrayIndex(i);
        u.save(pref);
        ++i;
    }

    pref.endArray();
    pref.endGroup();
}

void messages_widget::load()
{
    Preferences pref;
    pref.beginGroup("ED2KFriends");
    int size = pref.beginReadArray("Friends");

    for(int i = 0; i < size; ++i)
    {
        pref.setArrayIndex(i);
        friends.push_back(USER(pref));
        model->appendRow(new QStandardItem(QIcon(":/emule/users/Friends1.ico"), friends.back().strName));
    }

    pref.endArray();
    pref.endGroup();
}

bool messages_widget::event(QEvent* e)
{
    if (e->type() == QEvent::WindowActivate || e->type() == QEvent::Show)
    {
        if (lastMessageTab == tabWidget->currentIndex())
        {
            emit stopMessageNotification();
            lastMessageTab = -1;
        }
    }
    return QWidget::event(e);
}

void messages_widget::closeCurrentTab()
{
    if (tabWidget->currentIndex() >= 0)
        closeTab(tabWidget->currentIndex());
}

bool messages_widget::eventFilter(QObject *obj, QEvent *e)
 {
     if (obj == textMsg) 
     {
         if (e->type() == QEvent::KeyPress) 
         {
             QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
             if ( (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) &&
                  (keyEvent->modifiers() == Qt::ControlModifier) )
             {
                 pushMessage();
                 return true;
             }
             return false;
         }
         else
         {
             return false;
         }
     } 
     else 
     {
         return QWidget::eventFilter(obj, e);
     }
 }

void messages_widget::peerConnected(const libed2k::net_identifier& np, const QString& hash, bool bActive)
{
    std::vector<USER>::iterator it = findUser(np);
    if ( it != users.end())
    {
        if (it->connected >= 0)
        {
            addSystemMessage(it->edit, tr("*** Connected"));
        }
        else
        {
            addSystemMessage(it->edit, tr("*** Connection established"));
        }

        if (it->post_msg.length())
        {
            Preferences pref;
            addMessage(it->edit, pref.nick(), it->post_msg, greenColor);
            it->post_msg = "";
        }
        
        it->connected = 1;
    }

    if (!connectedPeers.contains(np))
        connectedPeers.append(np);

    setFriendIcon(np, true);
}

void messages_widget::peerDisconnected(const libed2k::net_identifier& np, const QString& hash, const libed2k::error_code ec)
{
    std::vector<USER>::iterator it = findUser(np);
    if ( it != users.end())
    {
        if (it->connected > 0)
        {
            it->connected = 0;
            addSystemMessage(it->edit, tr("*** Disconnected"));
        }
        else
        {
            addSystemMessage(it->edit, tr("*** Connection refused"));
        }
        it->post_msg = "";
    }
    else
    {
        if (connectedPeers.contains(np))
            connectedPeers.removeOne(np);
    }

    setFriendIcon(np, false);
}

void messages_widget::requestUserDirs()
{
    QModelIndex index = listFriends->currentIndex();

    if (!index.isValid())
        return;

    int num = index.row();
    QED2KPeerHandle::getPeerHandle(friends[num].netPoint).requestDirs();
}

void messages_widget::enableButtons(bool enable)
{
    btnSend->setEnabled(enable);
    btnClose->setEnabled(enable);
}

void messages_widget::addMessage(QTextEdit* edit, const QString& name, const QString& msg, const QString& color)
{
    QString htmlText = "<tr><td>&#91;" + QDateTime::currentDateTime().toString("hh:mm") + "&#93;" + "<font color='" + color + "'>&#x200D;" +
                       name + ": &#x200D;</font><font color='#000000'>" + msg + "</font></td></tr>";
    edit->moveCursor(QTextCursor::End);
    edit->insertHtml(htmlText);
    edit->moveCursor(QTextCursor::End);
}

void messages_widget::addSystemMessage(QTextEdit* edit, const QString& msg)
{
    QString htmlText = "<tr><td>&#91;" + QDateTime::currentDateTime().toString("hh:mm") + "&#93;" + "<font color='" + systemColor + "'>&#x200D;" +
                       msg + "&#x200D;</font></td></tr>";
    edit->moveCursor(QTextCursor::End);
    edit->insertHtml(htmlText);
    edit->moveCursor(QTextCursor::End);
}

std::vector<USER>::iterator messages_widget::findUser(const libed2k::net_identifier& np)
{
    std::vector<USER>::iterator it;
    for (it = users.begin(); it != users.end(); ++it)
        if (it->netPoint == np)
            break;

    return it;
}

void messages_widget::setFriendIcon(const libed2k::net_identifier& np, bool connected)
{
    std::vector<USER>::iterator it;
    int num = 0;
    for (it = friends.begin(); it != friends.end(); ++it, ++num)
        if (it->netPoint == np)
            break;

    if (it != friends.end())
    {
        if (connected)
        {
            it->connected = 1;
            model->item(num)->setIcon(QIcon(":/emule/users/Friends3.ico"));
        }
        else
        {
            if (it->connected < 0)
                model->item(num)->setIcon(QIcon(":/emule/users/Friends1.ico"));
            else
                model->item(num)->setIcon(QIcon(":/emule/users/Friends2.ico"));
        }
    }
}

void messages_widget::friendSelected(const QModelIndex& index, const QModelIndex& prev)
{
    if (!index.isValid())
        return;

    int num = index.row();
    QED2KPeerHandle peer = QED2KPeerHandle::findPeerHandle(friends[num].netPoint);

    label_name->setText(friends[num].strName);
    label_IP->setText(QString::fromStdString(libed2k::int2ipstr(friends[num].netPoint.m_nIP)));
    label_port->setText(QString::number(friends[num].netPoint.m_nPort));

    libed2k::peer_connection_options options = peer.getConnectionOptions();
    if (options.m_strModVersion.length())
        label_programm->setText(QString::fromStdString(options.m_strModVersion));
    else
        label_programm->setText("-");

    libed2k::md4_hash hash = peer.getHash();
    libed2k::md4_hash empty_hash;
    if (hash != empty_hash)
        label_hash->setText(QString::fromStdString(hash.toString()));
    else
        label_hash->setText("-");
}
