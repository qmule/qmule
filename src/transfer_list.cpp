#include "transport/session.h"
#include "transferlistwidget.h"
#include "peerlistwidget.h"
#include "torrentmodel.h"
#include "mainwindow.h"
#include "transfer_list.h"
#include "iconprovider.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QSpacerItem>
#include <QToolBar>

transfer_list::transfer_list(QWidget *parent, MainWindow *mainWindow)
    : QMainWindow(parent)
{
    btnText << tr("Download") << tr("Download") << tr("Upload") << tr("Download");

    hSplitter = new QSplitter(Qt::Vertical);
    hSplitter->setChildrenCollapsible(false);
    hSplitter->setContentsMargins(0, 0, 0, 0);

    setCentralWidget(hSplitter);

    verticalLayoutWidget1 = new QWidget(hSplitter);
    verticalLayoutWidget1->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
    verticalLayoutWidget2 = new QWidget(hSplitter);
    verticalLayoutWidget2->setObjectName(QString::fromUtf8("verticalLayoutWidget"));

    vboxLayout1 = new QVBoxLayout(verticalLayoutWidget1);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    vboxLayout1->setContentsMargins(0, 0, 0, 0);
    vboxLayout1->setSpacing(0);

    vboxLayout2 = new QVBoxLayout(verticalLayoutWidget2);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    vboxLayout2->setContentsMargins(0, 0, 0, 0);
    vboxLayout2->setSpacing(0);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    hboxLayout1->setContentsMargins(0, 0, 0, 0);
    hboxLayout1->setSpacing(0);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    hboxLayout2->setContentsMargins(0, 0, 0, 0);
    hboxLayout2->setSpacing(0);

    icons = new QIcon[topRowBtnCnt];
    icons[0].addFile(QString::fromUtf8(":/emule/transfer_list/SplitWindow.png"), QSize(), QIcon::Normal, QIcon::Off);
    icons[1].addFile(QString::fromUtf8(":/emule/transfer_list/DownloadFiles.png"), QSize(), QIcon::Normal, QIcon::Off);
    icons[2].addFile(QString::fromUtf8(":/emule/transfer_list/Upload.png"), QSize(), QIcon::Normal, QIcon::Off);
    icons[3].addFile(QString::fromUtf8(":/emule/transfer_list/Download.png"), QSize(), QIcon::Normal, QIcon::Off);

    btnSwitch = new QPushButton("", this);
    btnSwitch->setFlat(true);
    btnSwitch->setIcon(icons[0]);
    btnSwitch->setText(btnText[0]);
    //btnSwitch->setFixedSize(200, 24);

    btnSwitch2 = new QPushButton("", this);
    btnSwitch2->setFlat(true);
    btnSwitch2->setIcon(icons[3]);
    btnSwitch2->setText(btnText[3]);
    //btnSwitch->setFixedSize(200, 24);

    topRowButtons = new QPushButton*[topRowBtnCnt];
    for (int ii = 0; ii < topRowBtnCnt; ii++)
    {
        topRowButtons[ii] = createFlatButton(icons[ii]);
        topRowButtons[ii]->setToolTip(btnText[ii]);
    }
    topRowButtons[0]->setToolTip(tr("Split window"));

    bottomRowButtons = new QPushButton*[bottomRowBtnCnt];
    for (int ii = 0; ii < bottomRowBtnCnt; ii++)
    {
        bottomRowButtons[ii] = createFlatButton(icons[ii + 2]);
        bottomRowButtons[ii]->setToolTip(btnText[ii + 2]);
    }

    horizontalSpacer = new QSpacerItem(40, 2, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalSpacer2 = new QSpacerItem(40, 2, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addWidget(btnSwitch);
    hboxLayout1->addWidget(topRowButtons[0]);
    hboxLayout1->addWidget(topRowButtons[1]);
    hboxLayout1->addWidget(topRowButtons[2]);
    hboxLayout1->addWidget(topRowButtons[3]);
    hboxLayout1->addItem(horizontalSpacer);

    topRowButtons[0]->setChecked(true);

    hboxLayout2->addWidget(btnSwitch2);
    hboxLayout2->addWidget(bottomRowButtons[0]);
    hboxLayout2->addWidget(bottomRowButtons[1]);
    hboxLayout2->addItem(horizontalSpacer2);

    bottomRowButtons[1]->setChecked(true);

    transferList = new TransferListWidget(this, mainWindow, Session::instance());
    peersList = new PeerListWidget(this);
    peersList->showDownload();

    hSplitter->addWidget(verticalLayoutWidget1);
    hSplitter->addWidget(verticalLayoutWidget2);
    
    vboxLayout1->addLayout(hboxLayout1);  
    vboxLayout1->addWidget(transferList);  

    vboxLayout2->addLayout(hboxLayout2);  
    vboxLayout2->addWidget(peersList);  
    transferList->getSourceModel()->populate();

    connect(btnSwitch, SIGNAL(clicked()), this, SLOT(btnSwitchClick()));
    connect(btnSwitch2, SIGNAL(clicked()), this, SLOT(btnSwitchClick2()));

    for (int ii = 0; ii < topRowBtnCnt; ii++)
        connect(topRowButtons[ii], SIGNAL(clicked()), this, SLOT(btnTopClick()));

    for (int ii = 0; ii < bottomRowBtnCnt; ii++)
        connect(bottomRowButtons[ii], SIGNAL(clicked()), this, SLOT(btnBottomClick()));

    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshPeers()));
    refreshTimer->start(3000);

    currTopWidget = transferList;
    currBottomWidget = peersList;

    mainToolBar = new QToolBar(this);
    mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
    mainToolBar->setAcceptDrops(false);
    mainToolBar->setMovable(true);
    mainToolBar->setAllowedAreas(Qt::AllToolBarAreas);
    mainToolBar->setIconSize(QSize(24, 24));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainToolBar->setFloatable(false);
    setContextMenuPolicy(Qt::NoContextMenu);

    actionOpen = new QAction(this);
    actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
    actionOpen->setToolTip(tr("Add torrent file"));
    actionOpen->setIcon(IconProvider::instance()->getIcon("list-add"));
    //actionOpen->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+O")));

    actionDelete = new QAction(this);
    actionDelete->setObjectName(QString::fromUtf8("actionDelete"));
    actionDelete->setToolTip(tr("Delete"));
    actionDelete->setIcon(IconProvider::instance()->getIcon("list-remove"));

    actionStart = new QAction(this);
    actionStart->setObjectName(QString::fromUtf8("actionStart"));
    actionStart->setToolTip(tr("Resume"));
    actionStart->setIcon(IconProvider::instance()->getIcon("media-playback-start"));

    actionPause = new QAction(this);
    actionPause->setObjectName(QString::fromUtf8("actionPause"));
    actionPause->setToolTip(tr("Pause"));
    actionPause->setIcon(IconProvider::instance()->getIcon("media-playback-pause"));

    mainToolBar->addAction(actionOpen);
    mainToolBar->addAction(actionDelete);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionStart);
    mainToolBar->addAction(actionPause);
    addToolBar(Qt::LeftToolBarArea, mainToolBar);

    connect(actionOpen, SIGNAL(triggered()), mainWindow, SLOT(on_actionOpen_triggered()));
    connect(actionDelete, SIGNAL(triggered()), transferList, SLOT(deleteSelectedTorrents()));
    connect(actionStart, SIGNAL(triggered()), transferList, SLOT(startSelectedTorrents()));
    connect(actionPause, SIGNAL(triggered()), transferList, SLOT(pauseSelectedTorrents()));
      
    connect(peersList, SIGNAL(sendMessage(const QString&, const libed2k::net_identifier&)), this, SLOT(sendMessageToPeer(const QString&, const libed2k::net_identifier&)));
    connect(peersList, SIGNAL(addFriend(const QString&, const libed2k::net_identifier&)), this, SLOT(addPeerToFriends(const QString&, const libed2k::net_identifier&)));
}

transfer_list::~transfer_list()
{
    delete[] icons;
    delete[] topRowButtons;
    delete[] bottomRowButtons;

    delete hboxLayout1;
    delete hboxLayout2;
    delete vboxLayout1;
    delete vboxLayout2;
    delete verticalLayoutWidget1;
    delete verticalLayoutWidget2;
    delete hSplitter;

    delete refreshTimer;
}

QPushButton* transfer_list::createFlatButton(QIcon& icon)
{
    QPushButton* newButton = new QPushButton("", this);
    newButton->setCheckable(true);
    newButton->setFlat(true);    
    newButton->setIcon(icon);
    newButton->setFixedSize(24, 24);

    return newButton;
}

void transfer_list::btnSwitchClick()
{
    int ii = 0;
    for (; ii < topRowBtnCnt; ii++)
        if (topRowButtons[ii]->isChecked())
            break;
    topRowButtons[ii]->setChecked(false);
    ++ii;
    ii = ii % topRowBtnCnt;
    topRowButtons[ii]->setChecked(true);
    btnSwitch->setIcon(icons[ii]);
    btnSwitch->setText(btnText[ii]);

    ProcessTopButton(ii);
}

void transfer_list::btnSwitchClick2()
{
    int ii = 0;
    for (; ii < bottomRowBtnCnt; ii++)
        if (bottomRowButtons[ii]->isChecked())
            break;
    bottomRowButtons[ii]->setChecked(false);
    ++ii;
    ii = ii % bottomRowBtnCnt;
    bottomRowButtons[ii]->setChecked(true);
    btnSwitch2->setIcon(icons[ii + 2]);
    btnSwitch2->setText(btnText[ii + 2]);

    ProcessBottomButton(ii);
}

void transfer_list::btnTopClick()
{
    for (int ii = 0; ii < topRowBtnCnt; ii++)
    {
        if (topRowButtons[ii] == (QPushButton *)sender())
        {
            topRowButtons[ii]->setChecked(true);
            btnSwitch->setIcon(icons[ii]);
            btnSwitch->setText(btnText[ii]);
            ProcessTopButton(ii);
        }
        else
            topRowButtons[ii]->setChecked(false);
    }
}

void transfer_list::btnBottomClick()
{
    for (int ii = 0; ii < bottomRowBtnCnt; ii++)
    {
        if (bottomRowButtons[ii] == (QPushButton *)sender())
        {
            bottomRowButtons[ii]->setChecked(true);
            btnSwitch2->setIcon(icons[ii + 2]);
            btnSwitch2->setText(btnText[ii + 2]);
            ProcessBottomButton(ii);
        }
        else
            bottomRowButtons[ii]->setChecked(false);
    }
}

void transfer_list::ProcessTopButton(int btn_num)
{    
    if (btn_num != 0)
    {
        if (verticalLayoutWidget2->isVisible())
        {
            vboxLayout2->removeWidget(peersList);  
            vboxLayout1->addWidget(peersList);
            currTopWidget = NULL;
            verticalLayoutWidget2->hide();
        }

        switch (btn_num)
        {
            case 1:
            {
                if (currTopWidget != transferList)
                {
                    transferList->show();
                    peersList->hide();
                    currTopWidget = transferList;
                }
                break;
            }
            case 2:
            {
                if (currTopWidget != peersList)
                {
                    transferList->hide();
                    peersList->show();
                    currTopWidget = peersList;
                }
                peersList->showDownload(false);
                break;
            }
            case 3:
            {
                if (currTopWidget != peersList)
                {
                    transferList->hide();
                    peersList->show();
                    currTopWidget = peersList;
                }
                peersList->showDownload();
                break;
            }
            default:
            {
                transferList->hide();
                peersList->hide();
                currTopWidget = NULL;
            }
        }
    }
    else
    {
        if (verticalLayoutWidget2->isHidden())
        {
            if (currTopWidget)
                vboxLayout1->removeWidget(currTopWidget);  
            vboxLayout1->addWidget(transferList);
            vboxLayout2->addWidget(peersList);
            transferList->show();
            peersList->hide();
            if (currBottomWidget)
                currBottomWidget->show();
            currTopWidget = transferList;
            verticalLayoutWidget2->show();

            for (int ii = 0; ii < bottomRowBtnCnt; ii++)
            {
                if (bottomRowButtons[ii]->isChecked())
                {
                    ProcessBottomButton(ii);
                    break;
                }
            }
        }
    }
}

void transfer_list::ProcessBottomButton(int btn_num)
{
    QWidget* new_widget = NULL;
    bool switch_widgets = false;
    switch (btn_num)
    {
        case 0:
        {
            if (currBottomWidget != peersList)
            {
                switch_widgets = true;
                new_widget = peersList;                
            }
            peersList->showDownload(false);
            break;
        }
        case 1:
        {
            if (currBottomWidget != peersList)
            {
                switch_widgets = true;
                new_widget = peersList;                
            }
            peersList->showDownload();
            break;
        }
        default:
        {
            new_widget = NULL;
            switch_widgets = true;
        }
    }
    if (switch_widgets)
    {
        if (currBottomWidget)
        {
            currBottomWidget->hide();
        }

        if (new_widget)
            new_widget->show();
        currBottomWidget = new_widget;
    }
}

void transfer_list::refreshPeers()
{
    peersList->loadPeers();
}

void transfer_list::addPeerToFriends(const QString& user_name, const libed2k::net_identifier& np)
{
    emit addFriend(user_name, np);
}

void transfer_list::sendMessageToPeer(const QString& user_name, const libed2k::net_identifier& np)
{
    emit sendMessage(user_name, np);
}
