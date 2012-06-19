#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>

#include "search_widget.h"
#include "search_filter.h"
#include "preferences.h"

#include "libed2k/file.hpp"
#include "transport/session.h"

using namespace libed2k;

int selected_row(QAbstractItemView* view)
{
    QModelIndex index = view->currentIndex();
    return index.isValid() ? index.row() : -1;
}

QVariant selected_data(QAbstractItemView* view, int column)
{
    return view->model()->index(selected_row(view), column).data();
}

search_widget::search_widget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    tabSearch = new QTabBar(this);
    tabSearch->setObjectName(QString::fromUtf8("tabSearch"));
    tabSearch->setTabsClosable(true);
    tabSearch->setShape(QTabBar::RoundedNorth);
    tabSearch->setExpanding(false);
    tabSearch->hide();
    horizontalLayoutTabs->addWidget(tabSearch);

    menuResults = new QMenu(this);
    menuResults->setObjectName(QString::fromUtf8("menuStatus"));
    menuSubResults = new QMenu(this);
    menuSubResults->setObjectName(QString::fromUtf8("menuSubResults"));
    menuSubResults->setTitle(tr("Size"));
    
    closeAll = new QAction(this);
    closeAll->setObjectName(QString::fromUtf8("closeAll"));
    closeAll->setText(tr("Close all"));

    defValue = new QAction(this);
    defValue->setObjectName(QString::fromUtf8("defValue"));
    defValue->setText(tr("Default"));
    defValue->setCheckable(true);
    defValue->setChecked(true);

    defKilos = new QAction(this);
    defKilos->setObjectName(QString::fromUtf8("defKilos"));
    defKilos->setText(tr("kB"));
    defKilos->setCheckable(true);

    defMegas = new QAction(this);
    defMegas->setObjectName(QString::fromUtf8("defMegas"));
    defMegas->setText(tr("MB"));
    defMegas->setCheckable(true);

    menuSubResults->addAction(defValue);
    menuSubResults->addAction(defKilos);
    menuSubResults->addAction(defMegas);

    menuResults->addAction(closeAll);
    menuResults->addSeparator();
    menuResults->addMenu(menuSubResults);

    btnResults->setMenu(menuResults);

    QIcon icon1;
    icon1.addFile(QString::fromUtf8(":/emule/common/FileTypeAny.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon2;
    icon2.addFile(QString::fromUtf8(":/emule/common/FileTypeArchive.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon3;
    icon3.addFile(QString::fromUtf8(":/emule/common/FileTypeAudio.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon4;
    icon4.addFile(QString::fromUtf8(":/emule/common/FileTypeCDImage.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon5;
    icon5.addFile(QString::fromUtf8(":/emule/common/FileTypePicture.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon6;
    icon6.addFile(QString::fromUtf8(":/emule/common/FileTypeProgram.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon7;
    icon7.addFile(QString::fromUtf8(":/emule/common/FileTypeVideo.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon8;
    icon8.addFile(QString::fromUtf8(":/emule/common/FileTypeDocument.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon9;
    icon9.addFile(QString::fromUtf8(":/emule/common/FileTypeEmuleCollection.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon10;
    icon10.addFile(QString::fromUtf8(":/emule/common/FolderOpen.ico"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon11;
    icon11.addFile(QString::fromUtf8(":/emule/common/User.ico"), QSize(), QIcon::Normal, QIcon::Off);

    iconSerachActive.addFile(QString::fromUtf8(":/emule/search/SearchActive.png"), QSize(), QIcon::Normal, QIcon::Off);
    iconSearchResult.addFile(QString::fromUtf8(":/emule/search/SearchResult.png"), QSize(), QIcon::Normal, QIcon::Off);

    comboType->addItem(icon1, tr("Any"));
    comboType->addItem(icon2, tr("Archive"));
    comboType->addItem(icon3, tr("Audio"));
    comboType->addItem(icon4, tr("CD Image"));
    comboType->addItem(icon5, tr("Picture"));
    comboType->addItem(icon6, tr("Program"));
    comboType->addItem(icon7, tr("Video"));
    comboType->addItem(icon8, tr("Document"));
    comboType->addItem(icon9, tr("Emule Collection"));
    comboType->addItem(icon10, tr("Folder"));
    comboType->addItem(icon11, tr("User"));
    comboType->setMaxVisibleItems(11);

    tableCond->setEditTriggers(QAbstractItemView::AllEditTriggers);
    tableCond->setColumnWidth(0, 200);
    addCondRow();
    tableCond->item(0, 0)->setText(tr("Min. size [MB]"));
    addCondRow();
    tableCond->item(1, 0)->setText(tr("Max. size [MB]"));
    addCondRow();
    tableCond->item(2, 0)->setText(tr("Availability"));
    addCondRow();
    tableCond->item(3, 0)->setText(tr("Full sources"));
    addCondRow();
    tableCond->item(4, 0)->setText(tr("Extension"));
    addCondRow();
    tableCond->item(5, 0)->setText(tr("Codec"));
    addCondRow();
    tableCond->item(6, 0)->setText(tr("Min bitrait [kBit/sec]"));
    addCondRow();
    tableCond->item(7, 0)->setText(tr("Min duration [h:m:s]"));
    addCondRow();
    tableCond->item(8, 0)->setText(tr("Name"));
    tableCond->item(8, 0)->setFlags(Qt::NoItemFlags);
    tableCond->item(8, 1)->setFlags(Qt::NoItemFlags);
    addCondRow();
    tableCond->item(9, 0)->setText(tr("Artist"));
    tableCond->item(9, 0)->setFlags(Qt::NoItemFlags);
    tableCond->item(9, 1)->setFlags(Qt::NoItemFlags);
    addCondRow();
    tableCond->item(10, 0)->setText(tr("Album"));
    tableCond->item(10, 0)->setFlags(Qt::NoItemFlags);
    tableCond->item(10, 1)->setFlags(Qt::NoItemFlags);

    model = new QStandardItemModel(0, SWDelegate::SW_COLUMNS_NUM);
    model->setHeaderData(SWDelegate::SW_NAME, Qt::Horizontal,           tr("File Name"));
    model->setHeaderData(SWDelegate::SW_SIZE, Qt::Horizontal,           tr("File Size"));
    model->setHeaderData(SWDelegate::SW_AVAILABILITY, Qt::Horizontal,   tr("Availability"));
    model->setHeaderData(SWDelegate::SW_SOURCES, Qt::Horizontal,        tr("Sources"));
    model->setHeaderData(SWDelegate::SW_TYPE, Qt::Horizontal,           tr("Type"));
    model->setHeaderData(SWDelegate::SW_ID, Qt::Horizontal,             tr("ID"));
    model->setHeaderData(SWDelegate::SW_DURATION, Qt::Horizontal,       tr("Duration"));
    model->setHeaderData(SWDelegate::SW_BITRATE, Qt::Horizontal,        tr("Bitrate"));
    model->setHeaderData(SWDelegate::SW_CODEC, Qt::Horizontal,          tr("Codec"));

    filterModel = new QSortFilterProxyModel();
    filterModel->setDynamicSortFilter(true);
    filterModel->setSourceModel(model);
    filterModel->setFilterKeyColumn(SWDelegate::SW_NAME);
    filterModel->setFilterRole(Qt::DisplayRole);
    filterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    treeResult->setModel(filterModel);

    //proxyModel = new QSortFilterProxyModel();
    //proxyModel->setDynamicSortFilter(true);
    //proxyModel->setSourceModel(tableWidget_2->model());
    //tableWidget_2->setModel(proxyModel);
    itemDelegate = new SWDelegate(treeResult);
    treeResult->setItemDelegate(itemDelegate);

    searchFilter = new search_filter(this);
    searchFilter->setMinimumSize(QSize(180, 20));
    searchFilter->setMaximumSize(QSize(180, 16777215));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(searchFilter->sizePolicy().hasHeightForWidth());
    searchFilter->setSizePolicy(sizePolicy);

    searchFilter->hide();
    horizontalLayoutTabs->addWidget(searchFilter);

    connect(tableCond, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemCondClicked(QTableWidgetItem*)));
    connect(btnStart, SIGNAL(clicked()), this, SLOT(startSearch()));
    connect(btnMore, SIGNAL(clicked()), this, SLOT(continueSearch()));
    connect(Session::instance()->get_ed2k_session(), SIGNAL(searchResult(const libed2k::net_identifier&, const QString&, const std::vector<QED2KSearchResultEntry>&, bool)),
    		this, SLOT(processSearchResult(const libed2k::net_identifier&, const QString&, const std::vector<QED2KSearchResultEntry>&, bool)));
    connect(tabSearch, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(tabSearch, SIGNAL(currentChanged (int)), this, SLOT(selectTab(int)));
    connect(defValue,  SIGNAL(triggered()), this, SLOT(setSizeType()));
    connect(defKilos,  SIGNAL(triggered()), this, SLOT(setSizeType()));
    connect(defMegas,  SIGNAL(triggered()), this, SLOT(setSizeType()));
    connect(comboName,  SIGNAL(editTextChanged(const QString)), this, SLOT(searchTextChanged(const QString)));
    connect(comboName->lineEdit(), SIGNAL(returnPressed()), this, SLOT(startSearch()));
    connect(searchFilter, SIGNAL(textChanged(QString)), this, SLOT(applyFilter(QString)));
    connect(searchFilter, SIGNAL(filterSelected(SWDelegate::Column)), this, SLOT(setFilterType(SWDelegate::Column)));
    connect(btnDownload, SIGNAL(clicked()), this, SLOT(download()));

    userMenu = new QMenu(this);
    userMenu->setObjectName(QString::fromUtf8("userMenu"));
    userMenu->setTitle(tr("Clients"));

    userUpdate = new QAction(this);
    userUpdate->setObjectName(QString::fromUtf8("userUpdate"));
    userUpdate->setText(tr("Update client"));
    userUpdate->setIcon(QIcon(":/emule/users/userupdate.ico"));

    userDetails = new QAction(this);
    userDetails->setObjectName(QString::fromUtf8("userDetails"));
    userDetails->setText(tr("Details..."));
    userDetails->setIcon(QIcon(":/emule/users/UserDetails.ico"));

    userAddToFriends = new QAction(this);
    userAddToFriends->setObjectName(QString::fromUtf8("userAddToFriends"));
    userAddToFriends->setText(tr("Add to friends"));
    userAddToFriends->setIcon(QIcon(":/emule/users/UserAdd.ico"));

    userSendMessage = new QAction(this);
    userSendMessage->setObjectName(QString::fromUtf8("userSendMessage"));
    userSendMessage->setText(tr("Send message"));
    userSendMessage->setIcon(QIcon(":/emule/users/UserMessage.ico"));

    userBrowseFiles = new QAction(this);
    userBrowseFiles->setObjectName(QString::fromUtf8("userBrowseFiles"));
    userBrowseFiles->setText(tr("Browse files"));
    userBrowseFiles->setIcon(QIcon(":/emule/users/UserFiles.ico"));

    userMenu->addAction(userUpdate);
    userMenu->addAction(userDetails);
    userMenu->addAction(userAddToFriends);
    userMenu->addAction(userSendMessage);
    userMenu->addAction(userBrowseFiles);

    treeResult->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeResult, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayListMenu(const QPoint&)));
    connect(userUpdate,  SIGNAL(triggered()), this, SLOT(initPeer()));    
    connect(userSendMessage,  SIGNAL(triggered()), this, SLOT(sendMessage()));

    connect(Session::instance()->get_ed2k_session(),
    		SIGNAL(peerConnected(const libed2k::net_identifier&, const QString&, bool)),
            this, SLOT(peerConnected(const libed2k::net_identifier&, const QString&, bool)));
    connect(Session::instance()->get_ed2k_session(), SIGNAL(peerDisconnected(const libed2k::net_identifier& np, const QString&, const libed2k::error_code)),
            this, SLOT(peerDisconnected(const libed2k::net_identifier& np, const QString&, const libed2k::error_code)));
    connect(treeResult->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(resultSelectionChanged(const QItemSelection&, const QItemSelection&)));
    btnDownload->setEnabled(false);
}

search_widget::~search_widget()
{
    delete defValue;
    delete defKilos;
    delete defMegas;
    delete closeAll;
    delete menuSubResults;
    delete menuResults;
    delete itemDelegate;

    delete userUpdate;
    delete userDetails;
    delete userAddToFriends;
    delete userSendMessage;
    delete userBrowseFiles;
    delete userMenu;
}

void search_widget::addCondRow()
{
    int row = tableCond->rowCount();
    tableCond->insertRow(row);

    QTableWidgetItem *item0 = new QTableWidgetItem;
    item0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item0->setFlags(Qt::ItemIsEnabled);
    tableCond->setItem(row, 0, item0);

    QTableWidgetItem *item1 = new QTableWidgetItem;
    item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    tableCond->setItem(row, 1, item1);
    //tableCond->setCurrentItem(item0);
}

void search_widget::itemCondClicked(QTableWidgetItem* item)
{
    if (item->column() == 0)
    {
        tableCond->editItem(tableCond->item(item->row(), 1));
    }
}

void search_widget::startSearch()
{
    QString searchRequest = comboName->currentText();
    if (!searchRequest.length())
        return;

    bool bOk = false;
    int nMinSize = tableCond->item(0, 1)->text().toInt(&bOk);
    if (!bOk && tableCond->item(0, 1)->text().length() > 0)
    {
        showErrorParamMsg(0);
        return;
    }

    int nMaxSize = tableCond->item(1, 1)->text().toInt(&bOk);
    if (!bOk && tableCond->item(1, 1)->text().length() > 0)
    {
        showErrorParamMsg(1);
        return;
    }
    int nAvail = tableCond->item(2, 1)->text().toInt(&bOk);
    if (!bOk && tableCond->item(2, 1)->text().length() > 0)
    {
        showErrorParamMsg(2);
        return;
    }
    int nSources = tableCond->item(3, 1)->text().toInt(&bOk);
    if (!bOk && tableCond->item(3, 1)->text().length() > 0)
    {
        showErrorParamMsg(3);
        return;
    }
    QString fileExt = tableCond->item(4, 1)->text();
    QString mediaCodec = tableCond->item(5, 1)->text();
    int nBitRate = tableCond->item(6, 1)->text().toInt(&bOk);
    if (!bOk && tableCond->item(6, 1)->text().length() > 0)
    {
        showErrorParamMsg(6);
        return;
    }

    QString fileType = "";
    RESULT_TYPE resultType = RT_FILES;
    switch(comboType->currentIndex())
    {
        case 1:
            fileType = ED2KFTSTR_ARCHIVE.c_str();
            break;
        case 2:
            fileType = ED2KFTSTR_AUDIO.c_str();
            break;
        case 3:
            fileType = ED2KFTSTR_CDIMAGE.c_str();
            break;
        case 4:
            fileType = ED2KFTSTR_IMAGE.c_str();
            break;
        case 5:
            fileType = ED2KFTSTR_PROGRAM.c_str();
            break;
        case 6:
            fileType = ED2KFTSTR_VIDEO.c_str();
            break;
        case 7:
            fileType = ED2KFTSTR_DOCUMENT.c_str();
            break;
        case 8:
            fileType = ED2KFTSTR_EMULECOLLECTION.c_str();
            break;
        case 9:
            fileType = ED2KFTSTR_FOLDER.c_str();
            resultType = RT_FOLDERS;
            break;
        case 10:
            fileType = ED2KFTSTR_USER.c_str();
            resultType = RT_CLIENTS;
            break;
    }

    if (!tabSearch->count())
    {
        tabSearch->show();
        searchFilter->show();
    }

    nCurTabSearch = tabSearch->addTab(iconSerachActive, comboName->currentText());
    tabSearch->setCurrentIndex(nCurTabSearch);
    std::vector<QED2KSearchResultEntry> vec;
    SearchResult result(vec, resultType);
    searchItems.push_back(result);
    clearSearchTable();
    btnStart->setEnabled(false);
    btnCancel->setEnabled(false);
    btnMore->setEnabled(false);

    moreSearch = false;

    if (checkPlus->checkState() == Qt::Checked)
        searchRequest += " NOT +++";

    Session::instance()->get_ed2k_session()->searchFiles(searchRequest, nMinSize, nMaxSize, nAvail, nSources, fileType, fileExt, mediaCodec, nBitRate, 0);
}

void search_widget::continueSearch()
{
    btnStart->setEnabled(false);
    btnCancel->setEnabled(false);
    btnMore->setEnabled(false);

    tabSearch->setTabIcon(nCurTabSearch, iconSerachActive);

    moreSearch = true;
    Session::instance()->get_ed2k_session()->searchMoreResults();
}

void search_widget::processSearchResult(const libed2k::net_identifier& np,
		const QString& hash,
		const std::vector<QED2KSearchResultEntry>& vRes, bool bMoreResult)
{
    if (nCurTabSearch < 0)
        return;

    tabSearch->setTabIcon(nCurTabSearch, iconSearchResult);
    btnStart->setEnabled(true);      

    if (!moreSearch)
    {        
        btnMore->setEnabled(bMoreResult);
        searchItems[nCurTabSearch].vecResults.insert(searchItems[nCurTabSearch].vecResults.end(), vRes.begin(), vRes.end());
    }
    else
    {
        btnMore->setEnabled(bMoreResult);
        searchItems[nCurTabSearch].vecResults.insert(searchItems[nCurTabSearch].vecResults.end(), vRes.begin(), vRes.end());
    }

    if (tabSearch->currentIndex() == nCurTabSearch)
        selectTab(nCurTabSearch);
}

void search_widget::closeTab(int index)
{
    if (tabSearch->currentIndex() == nCurTabSearch)
    {
        nCurTabSearch = -1;
        btnMore->setEnabled(false);
    }
    tabSearch->removeTab(index);
    
    if (searchItems.size() > index)
        searchItems.erase(searchItems.begin() + index);
    
    if (!tabSearch->count())
    {
        clearSearchTable();
        searchFilter->hide();
    }
    else
        selectTab(tabSearch->currentIndex());
}

void search_widget::selectTab(int nTabNum)
{
    //btnMore->setEnabled(btnStart->isEnabled() && nTabNum == nCurTabSearch && morePossible);

    if (nTabNum >= searchItems.size() || nTabNum < 0)
        return;

    std::vector<QED2KSearchResultEntry> const& vRes = searchItems[nTabNum].vecResults;
    std::vector<QED2KSearchResultEntry>::const_iterator it;

    clearSearchTable();
    int row = 0;

    if (searchItems[nTabNum].resultType == RT_FILES)
    {
        for (it = vRes.begin(); it != vRes.end(); ++it)
        {
            model->insertRow(row);
            model->setData(model->index(row, SWDelegate::SW_NAME), it->m_strFilename);
            model->setData(model->index(row, SWDelegate::SW_SIZE), it->m_nFilesize);
            model->setData(model->index(row, SWDelegate::SW_AVAILABILITY), it->m_nCompleteSources);
            QString sources = QString::number(100 * it->m_nCompleteSources / it->m_nSources);
            sources += "%(";
            sources += QString::number(it->m_nSources);
            sources += ")";
            model->setData(model->index(row, SWDelegate::SW_SOURCES), sources);

            EED2KFileType fileType = GetED2KFileTypeID(it->m_strFilename.toStdString());
            switch (fileType)
            {
                case ED2KFT_AUDIO:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("Audio"));
                    break;
                case ED2KFT_VIDEO:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("Video"));
                    break;
                case ED2KFT_IMAGE:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("Picture"));
                    break;
                case ED2KFT_PROGRAM:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("Program"));
                    break;
                case ED2KFT_DOCUMENT:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("Document"));
                    break;
                case ED2KFT_ARCHIVE:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("Archive"));
                    break;
                case ED2KFT_CDIMAGE:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("CD Image"));
                    break;
                case ED2KFT_EMULECOLLECTION:
                    model->setData(model->index(row, SWDelegate::SW_TYPE), tr("Emule Collection"));
                    break;
                default:
                    // should set model any?
                    break;
            }
            model->setData(model->index(row, SWDelegate::SW_ID), it->m_hFile);
            if (fileType == ED2KFT_AUDIO || fileType == ED2KFT_VIDEO)
            {
                model->setData(model->index(row, SWDelegate::SW_DURATION), it->m_nMediaLength);
                model->setData(model->index(row, SWDelegate::SW_BITRATE), it->m_nMediaBitrate);
            }
            if (fileType == ED2KFT_VIDEO)
                model->setData(model->index(row, SWDelegate::SW_CODEC), it->m_strMediaCodec);
            row++;
        }
    }
    else if (searchItems[nTabNum].resultType == RT_CLIENTS)
    {
        QIcon user_icon(":/emule/common/client_red.ico");
        QIcon conn_icon(":/emule/common/User.ico");

        for (it = vRes.begin(); it != vRes.end(); ++it)
        {
            model->insertRow(row);
            QString user_name = it->m_strFilename;
            user_name.replace("+++USERNICK+++", "");
            model->setData(model->index(row, SWDelegate::SW_NAME), user_name.trimmed());
            model->setData(model->index(row, SWDelegate::SW_SIZE), it->m_nFilesize);
            model->setData(model->index(row, SWDelegate::SW_ID), it->m_hFile);

            std::vector<libed2k::net_identifier>::const_iterator con_ip;
            for (con_ip = connectedPeers.begin(); con_ip != connectedPeers.end(); ++con_ip)
                if (*con_ip == it->m_network_point)
                    break;

            if (con_ip == connectedPeers.end())
                model->item(row)->setIcon(user_icon);
            else
                model->item(row)->setIcon(conn_icon);
        }
    }

    QHeaderView* header = treeResult->header();
    model->sort(header->sortIndicatorSection(), header->sortIndicatorOrder());
}

void search_widget::clearSearchTable()
{
    //model->clear();
    //tableResult->clear();
    for (int ii = model->rowCount()-1; ii >= 0; --ii)
        model->removeRow(ii);
}

void search_widget::setSizeType()
{
    QObject* sender = QObject::sender();
    if (sender == defKilos)
    {
        defMegas->setChecked(false);
        defValue->setChecked(false);

        itemDelegate->setSizeType(misc::ST_KiB);
    }
    else if (sender == defMegas)
    {
        defKilos->setChecked(false);
        defValue->setChecked(false);

        itemDelegate->setSizeType(misc::ST_MiB);
    }
    else
    {
        defKilos->setChecked(false);
        defMegas->setChecked(false);

        itemDelegate->setSizeType(misc::ST_DEFAULT);
    }

    //tableResult->update(tableResult->rect());
    //qApp->processEvents();

    int width = treeResult->columnWidth(0);
    treeResult->setColumnWidth(0, width - 1);
    treeResult->setColumnWidth(0, width);
}

void search_widget::showErrorParamMsg(int numParam)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("Search request parsing error"));
    QString errorText(tr("Syntax error in '"));
    errorText += tableCond->item(numParam, 0)->text();
    errorText += tr("' parameter");
    msgBox.setInformativeText(errorText);
    msgBox.exec();
}

void search_widget::searchTextChanged(const QString text)
{
    btnStart->setEnabled(text.length() > 0);
}

void search_widget::applyFilter(QString filter)
{
    if (searchFilter->isFilterSet())
        filterModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive));
}

void search_widget::setFilterType(SWDelegate::Column column)
{
    filterModel->setFilterKeyColumn(column);
}

void search_widget::displayListMenu(const QPoint&) 
{
    if (tabSearch->currentIndex() < 0)
        return;

    if (searchItems[tabSearch->currentIndex()].resultType == RT_CLIENTS)
    {
        userDetails->setEnabled(false);
        userAddToFriends->setEnabled(false);
        userSendMessage->setEnabled(false);
        userBrowseFiles->setEnabled(false);

        userUpdate->setEnabled(true);

        QED2KSearchResultEntry entry;
        if (findSelectedUser(entry))
        {
            std::vector<libed2k::net_identifier>::const_iterator con_ip;
            for (con_ip = connectedPeers.begin(); con_ip != connectedPeers.end(); ++con_ip)
                if (*con_ip == entry.m_network_point)
                {
                    userDetails->setEnabled(true);
                    userAddToFriends->setEnabled(true);
                    userSendMessage->setEnabled(true);
                    userBrowseFiles->setEnabled(true);
                    userUpdate->setEnabled(false);
                    break;
                }
        }

        userMenu->exec(QCursor::pos());
    }
}

void search_widget::initPeer()
{
    QED2KSearchResultEntry entry;
    if (findSelectedUser(entry))
    {
        Session::instance()->get_ed2k_session()->initializePeer(entry.m_network_point);
    }
}

void search_widget::sendMessage()
{
    QED2KSearchResultEntry entry;
    if (findSelectedUser(entry))
    {
        emit sendMessage(entry.m_strFilename.replace("+++USERNICK+++", "").trimmed(), entry.m_network_point);
    }
}

void search_widget::peerConnected(const libed2k::net_identifier& np, const QString& hash, bool bActive)
{
    connectedPeers.push_back(np);

    QIcon conn_icon(":/emule/common/User.ico");

    setUserPicture(np, conn_icon);
}

void search_widget::peerDisconnected(const libed2k::net_identifier& np, const QString& hash, const libed2k::error_code ec)
{
	connectedPeers.erase(std::remove(connectedPeers.begin(), connectedPeers.end(), np), connectedPeers.end());
    QIcon user_icon(":/emule/common/client_red.ico");
    setUserPicture(np, user_icon);
}

void search_widget::resultSelectionChanged(const QItemSelection& sel, const QItemSelection& unsel)
{
    btnDownload->setEnabled(!sel.indexes().empty());
}

void search_widget::download()
{
    if (selected_row(treeResult) < 0)
    {
        ERR("download button should be disabled when result isn't selected");
        return;
    }

    QString filename = selected_data(treeResult, SWDelegate::SW_NAME).toString();
    QString filepath = QDir(Preferences().getSavePath()).filePath(filename);

    libed2k::add_transfer_params params;
    params.file_hash = libed2k::md4_hash::fromString(selected_data(treeResult, SWDelegate::SW_ID).toString().toStdString());
    params.file_path = filepath.toLocal8Bit().constData();
    params.file_size = selected_data(treeResult, SWDelegate::SW_SIZE).toULongLong();
    params.seed_mode = false;
    Session::instance()->addTransfer(params);
}

void search_widget::setUserPicture(const libed2k::net_identifier& np, QIcon& icon)
{
    if (tabSearch->currentIndex() < 0)
        return;

    if (searchItems[tabSearch->currentIndex()].resultType == RT_CLIENTS)
    {
        std::vector<QED2KSearchResultEntry> const& vRes = searchItems[tabSearch->currentIndex()].vecResults;
        std::vector<QED2KSearchResultEntry>::const_iterator it;

        for (it = vRes.begin(); it != vRes.end(); ++it)
        {
            if (it->m_network_point == np)
            {
                for (int row = 0; row < model->rowCount(); row++)
                    if (it->m_hFile == model->data(model->index(row, SWDelegate::SW_ID)).toString())
                    {
                        model->item(row)->setIcon(icon);
                        return;
                    }
                return;
            }
        }
    }

}

bool search_widget::findSelectedUser(QED2KSearchResultEntry& entry)
{
    if (selected_row(treeResult) < 0 || tabSearch->currentIndex() < 0)
        return false;

    std::vector<QED2KSearchResultEntry> const& vRes = searchItems[tabSearch->currentIndex()].vecResults;
    std::vector<QED2KSearchResultEntry>::const_iterator it = vRes.end();

    QString user_hash = selected_data(treeResult, SWDelegate::SW_ID).toString();

    for (it = vRes.begin(); it != vRes.end(); ++it)
    {
        if (it->m_hFile == user_hash)
        {
            entry = *it;
            return true;
        }
    }    
    return false;
}
