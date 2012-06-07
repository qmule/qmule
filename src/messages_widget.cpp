#include <QDateTime>
#include <QMenu>
#include <QPlainTextEdit>
#include <QStandardItemModel>

#include "libed2k/util.hpp"
#include "libed2k/session.hpp"
#include "transport/session.h"

#include "add_friend.h"
#include "messages_widget.h"

using namespace libed2k;

messages_widget::messages_widget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    imgFriends->setPixmap(QIcon(":/emule/users/Friend.ico").pixmap(16, 16));
    imgMessages->setPixmap(QIcon(":/emule/users/Message.ico").pixmap(16, 16));
    
    label_name->setText("-");
    label_hash->setText("-");
    label_programm->setText("-");
    label_ident->setText("-");
    label_upload->setText("-");
    label_download->setText("-");

    QList<int> sizes;
    sizes.append(300);
    sizes.append(1000000);
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    userMenu = new QMenu(this);
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

    model = new QStandardItemModel(0, 1);
    listFriends->setModel(model);
    listFriends->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(listFriends, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayListMenu(const QPoint&)));
    connect(userAdd, SIGNAL(triggered()), this, SLOT(addFriend()));
    connect(userSendMessage, SIGNAL(triggered()), this, SLOT(sendMessage()));
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(btnSend, SIGNAL(clicked()), this, SLOT(pushMessage()));
    connect(Session::instance()->get_ed2k_session(), SIGNAL(peerMessage(const libed2k::net_identifier&, const QString&, const QString&)), this, SLOT(newMessage(const libed2k::net_identifier&, const QString&, const QString&)));
    connect(Session::instance()->get_ed2k_session(), SIGNAL(peerCaptchaRequest(const libed2k::net_identifier&, const QString&, const QPixmap&)), this, SLOT(peerCaptchaRequest(const libed2k::net_identifier&, const QString&, const QPixmap&)));
    connect(Session::instance()->get_ed2k_session(), SIGNAL(peerCaptchaResult(const libed2k::net_identifier&, const QString&, quint8)), this, SLOT(peerCaptchaResult(const libed2k::net_identifier&, const QString&, quint8)));
}

messages_widget::~messages_widget()
{
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
}

void messages_widget::startChat(const QString& user_name, const libed2k::net_identifier& np)
{
    QTextEdit* edit = new QTextEdit(this);
    edit->setReadOnly(true);
    tabWidget->addTab(edit, user_name);

    USER user;
    user.strName = user_name;
    user.netPoint = np;
    users.push_back(user);
}

void messages_widget::pushMessage()
{
    QString msg = textMsg->toPlainText();
    textMsg->clear();
    if (tabWidget->currentIndex() < 0 || !msg.length())
        return;
    USER user = users[tabWidget->currentIndex()];

    QTextEdit* edit = qobject_cast<QTextEdit*>(tabWidget->widget(tabWidget->currentIndex()));

    QDateTime date_time = QDateTime::currentDateTime();
    QString newMsg = "[" + date_time.toString("hh:mm") + "] " + "me: " + msg;

    edit->append(newMsg);

    Session::instance()->get_ed2k_session()->sendMessageToPeer(user.netPoint, msg);
}

void messages_widget::newMessage(const libed2k::net_identifier& np, const QString& hash, const QString& strMessage)
{
    std::vector<USER>::const_iterator it;
    int nTab = 0;
    for (it = users.begin(); it != users.end(); ++it, ++nTab)
    {
        if (it->netPoint == np)
            break;
    }
    if (it != users.end())
    {
        QTextEdit* edit = qobject_cast<QTextEdit*>(tabWidget->widget(nTab));
        QDateTime date_time = QDateTime::currentDateTime();
        QString msg = "[" + date_time.toString("hh:mm") + "] " + it->strName + ": " + strMessage;
        edit->append(msg);
    }
}

void messages_widget::peerCaptchaRequest(const libed2k::net_identifier& np, const QString& hash, const QPixmap& pm)
{
    std::vector<USER>::const_iterator it;
    int nTab = 0;
    for (it = users.begin(); it != users.end(); ++it, ++nTab)
    {
        if (it->netPoint == np)
            break;
    }
    if (it != users.end())
    {
        QTextEdit* edit = qobject_cast<QTextEdit*>(tabWidget->widget(nTab));
        QTextCursor cursor = edit->textCursor();
        //QTextImageFormat imageFormat;
        //imageFormat.setWidth( 16 );
        //imageFormat.setHeight( 16 );
        //imageFormat.setName( ":/emule/common/client_red.ico" );

        cursor.insertText(tr("\nTo avoid spam user is asking for captcha autentification. Please enter symbols on the picture below: "));
        cursor.insertImage(pm.toImage());
        //cursor.insertImage(imageFormat);
    }
    //captcha* dlg = new captcha(pm, this);
    //dlg->exec();
}

void messages_widget::peerCaptchaResult(const libed2k::net_identifier& np, const QString& hash, quint8 nResult)
{
    std::vector<USER>::const_iterator it;
    int nTab = 0;
}

void messages_widget::displayListMenu(const QPoint& pos) 
{
    QItemSelectionModel* model = listFriends->selectionModel();
    QModelIndexList selectedIndexes = model->selectedRows();

    if (selectedIndexes.size() == 0)
    {
        userSendMessage->setEnabled(false);;
        userDetails->setEnabled(false);;
        userBrowseFiles->setEnabled(false);;
        userDelete->setEnabled(false);;
    }
    else
    {
        userSendMessage->setEnabled(true);;
        userDetails->setEnabled(true);;
        userBrowseFiles->setEnabled(true);;
        userDelete->setEnabled(true);;
    }

    userMenu->exec(pos);
}

void messages_widget::addFriend()
{
    add_friend dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        USER new_friend;
        new_friend.strName = dlg.getName();
        if (!new_friend.strName.length())
            new_friend.strName = dlg.getIP();
        new_friend.netPoint.m_nIP = address2int(ip::address::from_string(dlg.getIP().toUtf8().constData()));
        new_friend.netPoint.m_nPort = dlg.getPort().toInt();

        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 0), new_friend.strName);
        model->item(row)->setIcon(QIcon(":/emule/users/Friends1.ico"));

        friends.push_back(new_friend);
    }
}

void messages_widget::sendMessage()
{
    QItemSelectionModel* model = listFriends->selectionModel();
    QModelIndexList selectedIndexes = model->selectedRows();

    if (selectedIndexes.size() == 0)
        return;

    int num = selectedIndexes[0].row();

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
