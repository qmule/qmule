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
#include "qed2kpeerhandle.h"

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

QVariant selected_data(QAbstractItemView* view, int column, const QModelIndex& index)
{
    return view->model()->index(index.row(), column, index.parent()).data();
}

search_widget::search_widget(QWidget *parent)
    : QWidget(parent) , nCurTabSearch(-1), nSortedColumn(-1)
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

    iconAny.addFile(QString::fromUtf8(":/emule/common/FileTypeAny.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconArchive.addFile(QString::fromUtf8(":/emule/common/FileTypeArchive.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconAudio.addFile(QString::fromUtf8(":/emule/common/FileTypeAudio.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconCDImage.addFile(QString::fromUtf8(":/emule/common/FileTypeCDImage.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconPicture.addFile(QString::fromUtf8(":/emule/common/FileTypePicture.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconProgram.addFile(QString::fromUtf8(":/emule/common/FileTypeProgram.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconVideo.addFile(QString::fromUtf8(":/emule/common/FileTypeVideo.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconDocument.addFile(QString::fromUtf8(":/emule/common/FileTypeDocument.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconCollection.addFile(QString::fromUtf8(":/emule/common/FileTypeEmuleCollection.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconFolder.addFile(QString::fromUtf8(":/emule/common/FolderOpen.ico"), QSize(), QIcon::Normal, QIcon::Off);
    iconUser.addFile(QString::fromUtf8(":/emule/common/User.ico"), QSize(), QIcon::Normal, QIcon::Off);

    iconSerachActive.addFile(QString::fromUtf8(":/emule/search/SearchActive.png"), QSize(), QIcon::Normal, QIcon::Off);
    iconSearchResult.addFile(QString::fromUtf8(":/emule/search/SearchResult.png"), QSize(), QIcon::Normal, QIcon::Off);
    iconUserFiles.addFile(QString::fromUtf8(":/emule/common/User.ico"), QSize(), QIcon::Normal, QIcon::Off);

    comboType->addItem(iconAny, tr("Any"));
    comboType->addItem(iconArchive, tr("Archive"));
    comboType->addItem(iconAudio, tr("Audio"));
    comboType->addItem(iconCDImage, tr("CD Image"));
    comboType->addItem(iconPicture, tr("Picture"));
    comboType->addItem(iconProgram, tr("Program"));
    comboType->addItem(iconVideo, tr("Video"));
    comboType->addItem(iconDocument, tr("Document"));
    comboType->addItem(iconCollection, tr("Emule Collection"));
    comboType->addItem(iconFolder, tr("Folder"));
    comboType->addItem(iconUser, tr("User"));
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

    model.reset(new QStandardItemModel(0, SWDelegate::SW_COLUMNS_NUM));
    model.data()->setHeaderData(SWDelegate::SW_NAME, Qt::Horizontal,           tr("File Name"));
    model.data()->setHeaderData(SWDelegate::SW_SIZE, Qt::Horizontal,           tr("File Size"));
    model.data()->setHeaderData(SWDelegate::SW_AVAILABILITY, Qt::Horizontal,   tr("Availability"));
    model.data()->setHeaderData(SWDelegate::SW_SOURCES, Qt::Horizontal,        tr("Sources"));
    model.data()->setHeaderData(SWDelegate::SW_TYPE, Qt::Horizontal,           tr("Type"));
    model.data()->setHeaderData(SWDelegate::SW_ID, Qt::Horizontal,             tr("ID"));
    model.data()->setHeaderData(SWDelegate::SW_DURATION, Qt::Horizontal,       tr("Duration"));
    model.data()->setHeaderData(SWDelegate::SW_BITRATE, Qt::Horizontal,        tr("Bitrate"));
    model.data()->setHeaderData(SWDelegate::SW_CODEC, Qt::Horizontal,          tr("Codec"));

    filterModel.reset(new QSortFilterProxyModel());
    filterModel.data()->setDynamicSortFilter(true);
    filterModel.data()->setSourceModel(model.data());
    filterModel.data()->setFilterKeyColumn(SWDelegate::SW_NAME);
    filterModel.data()->setFilterRole(Qt::DisplayRole);
    filterModel.data()->setSortCaseSensitivity(Qt::CaseInsensitive);

    treeResult->setModel(filterModel.data());

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
    connect(Session::instance()->get_ed2k_session(), SIGNAL(peerSharedDirectories(const libed2k::net_identifier&, const QString&, const QStringList&)),
            this, SLOT(processUserDirs(const libed2k::net_identifier&, const QString&, const QStringList&)));
    connect(Session::instance()->get_ed2k_session(),
            SIGNAL(peerSharedDirectoryFiles(const libed2k::net_identifier&, const QString&, const QString&, const std::vector<QED2KSearchResultEntry>&)),
            this, SLOT(processUserFiles(const libed2k::net_identifier&, const QString&, const QString&, const std::vector<QED2KSearchResultEntry>&)));

    void processUserFiles(const libed2k::net_identifier& np, const QString& hash,
                          const QString& strDirectory, const std::vector<QED2KSearchResultEntry>& vRes);


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
    connect(userBrowseFiles,  SIGNAL(triggered()), this, SLOT(requestUserDirs()));

    connect(Session::instance()->get_ed2k_session(),
    		SIGNAL(peerConnected(const libed2k::net_identifier&, const QString&, bool)),
            this, SLOT(peerConnected(const libed2k::net_identifier&, const QString&, bool)));
    connect(Session::instance()->get_ed2k_session(),
            SIGNAL(peerDisconnected(const libed2k::net_identifier&, const QString&, const libed2k::error_code)),
            this, SLOT(peerDisconnected(const libed2k::net_identifier&, const QString&, const libed2k::error_code)));
    connect(treeResult->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(resultSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(treeResult, SIGNAL(expanded(const QModelIndex&)), this, SLOT(itemExpanded(const QModelIndex&)));
    connect(treeResult, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(itemCollapsed(const QModelIndex&)));
    connect(treeResult->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(sortChanged(int, Qt::SortOrder)));
    
    btnDownload->setEnabled(false);
    // sort by name ascending
    treeResult->header()->setSortIndicator(SWDelegate::SW_NAME, Qt::AscendingOrder);
}

search_widget::~search_widget()
{

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

void search_widget::sortChanged(int logicalIndex, Qt::SortOrder order)
{
    if (nSortedColumn != logicalIndex && logicalIndex == SWDelegate::SW_AVAILABILITY)
    {
        nSortedColumn = logicalIndex;
        treeResult->header()->setSortIndicator(SWDelegate::SW_AVAILABILITY, Qt::DescendingOrder);
    }
    nSortedColumn = logicalIndex;
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
    QString reqType = tr("Files: ");
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
            reqType = tr("Folders: ");
            break;
        case 10:
            fileType = ED2KFTSTR_USER.c_str();
            resultType = RT_CLIENTS;
            reqType = tr("Clients: ");
            break;
    }

    if (!tabSearch->count())
    {
        tabSearch->show();
        searchFilter->show();
    }

    nCurTabSearch = tabSearch->addTab(iconSerachActive, reqType + comboName->currentText());
    tabSearch->setCurrentIndex(nCurTabSearch);
    
    std::vector<QED2KSearchResultEntry> vec;
    SearchResult result(comboName->currentText(), resultType, vec);
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

    quint64 overallSize = 0;
    QString strCaption;
    std::vector<QED2KSearchResultEntry>::const_iterator it;
    std::vector<QED2KSearchResultEntry> const& vecResults = searchItems[nCurTabSearch].vecResults;
    if (vRes.size() > 0)
    {
        switch (searchItems[nCurTabSearch].resultType)
        {
            case RT_FILES:
            {
                for (it = vecResults.begin(); it != vecResults.end(); ++it)
                    overallSize += it->m_nFilesize;
                strCaption = tr("Files: ");
                break;
            }
            case RT_FOLDERS:
            {
                for (it = vecResults.begin(); it != vecResults.end(); ++it)
                {
                    quint64 total_size = ((quint64)it->m_nMediaBitrate << 32) + (unsigned int)it->m_nMediaLength;
                    total_size = total_size ? total_size : it->m_nFilesize;
                    overallSize += total_size;
                }
                strCaption = tr("Folders: ");
                break;
            }
            case RT_CLIENTS:
            {
                for (it = vecResults.begin(); it != vecResults.end(); ++it)
                {
                    quint64 total_size = ((quint64)it->m_nMediaBitrate << 32) + (unsigned int)it->m_nMediaLength;
                    overallSize += total_size;
                }
                strCaption = tr("Clients: ");
                break;
            }
        }

        strCaption += searchItems[nCurTabSearch].strRequest + " (" + QString::number(qulonglong(vecResults.size())) + ") - " + misc::friendlyUnit(overallSize);
        tabSearch->setTabText(nCurTabSearch, strCaption);
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
    if (nTabNum >= searchItems.size() || nTabNum < 0)
        return;

    std::vector<QED2KSearchResultEntry> const& vRes = searchItems[nTabNum].vecResults;
    std::vector<QED2KSearchResultEntry>::const_iterator it;

    clearSearchTable();
    int row = 0;

    treeResult->setItemsExpandable(false);
    treeResult->setRootIsDecorated(false);
    treeResult->setSelectionMode(QAbstractItemView::MultiSelection);
    if (searchItems[nTabNum].resultType == RT_FILES)
    {
        for (it = vRes.begin(); it != vRes.end(); ++it)
        {
            model->insertRow(row);
            fillFileValues(row, *it);
            row++;
        }
    }
    else if (searchItems[nTabNum].resultType == RT_CLIENTS)
    {
        treeResult->setSelectionMode(QAbstractItemView::SingleSelection);
        QIcon user_icon(":/emule/common/client_red.ico");
        QIcon conn_icon(":/emule/common/User.ico");

        for (it = vRes.begin(); it != vRes.end(); ++it)
        {
            model->insertRow(row);
            QString user_name = it->m_strFilename;
            user_name.replace("+++USERNICK+++", "");
            // for users size calculated from
            // m_nMediaLength  - low part of real size
            // m_nMediaBitrate - high part of real size
            model->setData(model->index(row, SWDelegate::SW_NAME), user_name.trimmed());
            quint64 total_size = ((quint64)it->m_nMediaBitrate << 32) + (unsigned int)it->m_nMediaLength;
            model->setData(model->index(row, SWDelegate::SW_SIZE), total_size);
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
    else if (searchItems[nTabNum].resultType == RT_USER_DIRS)
    {
        treeResult->setItemsExpandable(true);
        treeResult->setRootIsDecorated(true);
        std::vector<UserDir>& userDirs = searchItems[nTabNum].vecUserDirs;
        std::vector<UserDir>::iterator dir_iter;
        for (dir_iter = userDirs.begin(); dir_iter != userDirs.end(); ++dir_iter)
        {
            model->insertRow(row);
            model->setData(model->index(row, SWDelegate::SW_NAME), dir_iter->dirPath);
            model->item(row)->setIcon(iconFolder);
            QModelIndex index = model->index(row, 0);
            row++;

            const std::vector<QED2KSearchResultEntry>& files = dir_iter->vecFiles;
            if (files.size() > 0)
            {
                model->setData(model->index(row, SWDelegate::SW_AVAILABILITY),
                               qulonglong(files.size()));
                quint64 size = 0;
                std::vector<QED2KSearchResultEntry>::const_iterator file_iter;
                for (file_iter = files.begin(); file_iter != files.end(); ++file_iter)
                    size += file_iter->m_nFilesize;
                model->setData(model->index(row, SWDelegate::SW_SIZE), size);

                model->insertRows(0, files.size(), index);
                model->insertColumns(0, 9, index);
            }

            dir_iter->bFilled = false;
            if (dir_iter->bExpanded)
                treeResult->setExpanded(filterModel->mapFromSource(index), true);
        }        
    }
    else if (searchItems[nTabNum].resultType == RT_FOLDERS)
    {
        treeResult->setItemsExpandable(true);
        treeResult->setRootIsDecorated(true);
        std::vector<UserDir>& userDirs = searchItems[nTabNum].vecUserDirs;

        for (it = vRes.begin(); it != vRes.end(); ++it)
        {
            UserDir dir;
            dir.dirPath = it->m_strFilename;
            userDirs.push_back(dir);
            
            model->insertRow(row);
            QString folderName = it->m_strFilename;
            int nPos = folderName.lastIndexOf("+++");
            if (nPos >= 0)
                folderName = folderName.right(folderName.length() - nPos - 3);
            
            nPos = folderName.lastIndexOf("ED2K--");
            if (nPos >= 0)
                folderName = "ED2K:\\" + folderName.right(folderName.length() - nPos - 6);

            nPos = folderName.indexOf(".emulecollection");
            int nQnty = 0;
            if (nPos >= 0)
            {
                folderName = folderName.left(nPos);
                nPos = folderName.lastIndexOf('-');
                if (nPos < 0)
                    nPos = folderName.lastIndexOf('\\');
                if (nPos >= 0)
                {
                    nQnty = folderName.right(folderName.length() - nPos - 1).toInt();
                    folderName = folderName.left(nPos);
                }
            }
            folderName.replace('-', '\\');

            model->setData(model->index(row, SWDelegate::SW_NAME), folderName);
            model->item(row)->setIcon(iconFolder);
            model->setData(model->index(row, SWDelegate::SW_AVAILABILITY), nQnty);
            quint64 total_size = ((quint64)it->m_nMediaBitrate << 32) + (unsigned int)it->m_nMediaLength;
            total_size = total_size ? total_size : it->m_nFilesize;
            model->setData(model->index(row, SWDelegate::SW_SIZE), total_size);
            model->setData(model->index(row, SWDelegate::SW_ID), it->m_hFile);

            QModelIndex index = model->index(row, 0);
            model->insertRows(0, 1, index);
            model->insertColumns(0, 9, index);
        }
    }

    QHeaderView* header = treeResult->header();
    model->sort(header->sortIndicatorSection(), header->sortIndicatorOrder());
}

void search_widget::fillFileValues(int row, const QED2KSearchResultEntry& fileEntry, const QModelIndex& parent)
{
    model->setData(model->index(row, SWDelegate::SW_NAME, parent), fileEntry.m_strFilename);
    model->setData(model->index(row, SWDelegate::SW_SIZE, parent), fileEntry.m_nFilesize);
    model->setData(model->index(row, SWDelegate::SW_AVAILABILITY, parent), fileEntry.m_nCompleteSources);
    QString sources = (fileEntry.m_nSources > 0) ? (QString::number(100 * fileEntry.m_nCompleteSources / fileEntry.m_nSources)) : "0";
    sources += "%(";
    sources += QString::number(fileEntry.m_nSources);
    sources += ")";
    model->setData(model->index(row, SWDelegate::SW_SOURCES, parent), sources);

    EED2KFileType fileType = GetED2KFileTypeID(fileEntry.m_strFilename.toStdString());
    switch (fileType)
    {
        case ED2KFT_AUDIO:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("Audio"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconAudio);
            break;
        case ED2KFT_VIDEO:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("Video"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconVideo);
            break;
        case ED2KFT_IMAGE:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("Picture"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconPicture);
            break;
        case ED2KFT_PROGRAM:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("Program"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconProgram);
            break;
        case ED2KFT_DOCUMENT:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("Document"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconDocument);
            break;
        case ED2KFT_ARCHIVE:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("Archive"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconArchive);
            break;
        case ED2KFT_CDIMAGE:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("CD Image"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconCDImage);
            break;
        case ED2KFT_EMULECOLLECTION:
            model->setData(model->index(row, SWDelegate::SW_TYPE, parent), tr("Emule Collection"));
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconCollection);
            break;
        default:
            model->itemFromIndex(model->index(row, 0, parent))->setIcon(iconAny);

    }
    model->setData(model->index(row, SWDelegate::SW_ID, parent), fileEntry.m_hFile);
    if (fileType == ED2KFT_AUDIO || fileType == ED2KFT_VIDEO)
    {
        model->setData(model->index(row, SWDelegate::SW_DURATION, parent), fileEntry.m_nMediaLength);
        model->setData(model->index(row, SWDelegate::SW_BITRATE, parent), fileEntry.m_nMediaBitrate);
    }
    if (fileType == ED2KFT_VIDEO)
        model->setData(model->index(row, SWDelegate::SW_CODEC, parent), fileEntry.m_strMediaCodec);
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
                    if (QED2KPeerHandle::getPeerHandle(entry.m_network_point).isAllowedSharedFilesView())
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
        QED2KPeerHandle::getPeerHandle(entry.m_network_point);
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
    if (tabSearch->currentIndex() >= 0 && (searchItems[tabSearch->currentIndex()].resultType == RT_CLIENTS ||
                                           searchItems[tabSearch->currentIndex()].resultType == RT_FOLDERS))
        btnDownload->setEnabled(false);
    else
        btnDownload->setEnabled(!sel.indexes().empty());
}

void search_widget::download()
{
    if (selected_row(treeResult) < 0)
    {
        ERR("download button should be disabled when result isn't selected");
        return;
    }

    bool bDirs = false;
    if (tabSearch->currentIndex() >= 0 && searchItems[tabSearch->currentIndex()].resultType == RT_USER_DIRS)
        bDirs = true;

    QModelIndexList selected = treeResult->selectionModel()->selectedIndexes();
    QModelIndexList::const_iterator iter;

    for (iter = selected.begin(); iter != selected.end(); ++iter)
    {
        if (bDirs && iter->parent() == treeResult->rootIndex())
            continue;
        QString filename = selected_data(treeResult, SWDelegate::SW_NAME, *iter).toString();
        QString filepath = QDir(Preferences().getSavePath()).filePath(filename);

        libed2k::add_transfer_params params;
        params.file_hash = libed2k::md4_hash::fromString(selected_data(treeResult, SWDelegate::SW_ID, *iter).toString().toStdString());
        params.file_path = filepath.toUtf8().constData();
        params.file_size = selected_data(treeResult, SWDelegate::SW_SIZE, *iter).toULongLong();
        params.seed_mode = false;
        Session::instance()->addTransfer(params);
    }
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

void search_widget::requestUserDirs()
{
    QED2KSearchResultEntry entry;
    if (findSelectedUser(entry))
    {
        QED2KPeerHandle::getPeerHandle(entry.m_network_point).requestDirs();
    }
}

void search_widget::processUserDirs(const libed2k::net_identifier& np, const QString& hash, const QStringList& strList)
{
    QED2KPeerHandle peer = QED2KPeerHandle::getPeerHandle(np);
    QString userName = peer.getUserName();

    if (!tabSearch->count())
    {
        tabSearch->show();
        searchFilter->show();
    }

    std::vector<QED2KSearchResultEntry> vec;
    std::vector<UserDir> vecUserDirs;

    QStringList::const_iterator constIterator;
    for (constIterator = strList.constBegin(); constIterator != strList.constEnd(); ++constIterator)
    {
        UserDir userDir;
        userDir.dirPath = (*constIterator);
        vecUserDirs.push_back(userDir);
    }

    SearchResult result(userName, RT_USER_DIRS, vec, vecUserDirs, np);
    searchItems.push_back(result);
    clearSearchTable();

    nCurTabSearch = tabSearch->addTab(iconUserFiles, tr("Files: ") + userName);
    tabSearch->setCurrentIndex(nCurTabSearch);

    for (constIterator = strList.constBegin(); constIterator != strList.constEnd(); ++constIterator)
    {
        peer.requestFiles(*constIterator);
    }


    btnMore->setEnabled(false);
}

void search_widget::processUserFiles(const libed2k::net_identifier& np, const QString& hash,
                                     const QString& strDirectory, const std::vector<QED2KSearchResultEntry>& vRes)
{
    int nTabCnt = searchItems.size();
    int nTabNum = 0;
    
    for (; nTabNum < nTabCnt; nTabNum++)
    {
        if (searchItems[nTabNum].netPoint == np)
            break;
    }

    if (nTabNum == nTabCnt)
        return;

    std::vector<UserDir>& userDirs = searchItems[nTabNum].vecUserDirs;
    std::vector<UserDir>::iterator iter;

    quint64 overallSize = 0;
    quint64 totalQnty = 0;
    for (iter = userDirs.begin(); iter != userDirs.end(); ++iter)
    {
        if (iter->dirPath == strDirectory)
        {
            iter->vecFiles.insert(iter->vecFiles.end(), vRes.begin(), vRes.end());
            break;
        }
        totalQnty += iter->vecFiles.size();
        std::vector<QED2KSearchResultEntry>::const_iterator file_iter;
        for (file_iter = iter->vecFiles.begin(); file_iter != iter->vecFiles.end(); ++file_iter)
            overallSize += file_iter->m_nFilesize;
    }
    QString strCaption = tr("Files: ") + searchItems[nTabNum].strRequest + " (" + QString::number(totalQnty) + ") - " + misc::friendlyUnit(overallSize);
    tabSearch->setTabText(nTabNum, strCaption);

    if (tabSearch->currentIndex() == nTabNum)
    {
        for (int row = 0; row < model->rowCount(); row++)
            if (strDirectory == model->data(model->index(row, SWDelegate::SW_NAME)).toString()&&
                iter->vecFiles.size() > 0)
            {
                model->setData(model->index(row, SWDelegate::SW_AVAILABILITY),
                               qulonglong(iter->vecFiles.size()));
                quint64 size = 0;
                std::vector<QED2KSearchResultEntry>::const_iterator file_iter;
                for (file_iter = iter->vecFiles.begin(); file_iter != iter->vecFiles.end(); ++file_iter)
                    size += file_iter->m_nFilesize;
                model->setData(model->index(row, SWDelegate::SW_SIZE), size);

                QModelIndex index = model->index(row, 0);
                model->insertRows(0, iter->vecFiles.size(), index);
                model->insertColumns(0, 9, index);
                break;
            }
    }
}

void search_widget::itemCollapsed(const QModelIndex& index)
{
    std::vector<UserDir>& userDirs = searchItems[tabSearch->currentIndex()].vecUserDirs;
    QString dirName = index.data().toString();
    std::vector<UserDir>::iterator iter;
    for (iter = userDirs.begin(); iter != userDirs.end(); ++iter)
    {
        if(iter->dirPath == dirName)
        {
            iter->bExpanded = false;
            break;
        }
    }
}

void search_widget::itemExpanded(const QModelIndex& index)
{
    if (searchItems[tabSearch->currentIndex()].resultType == RT_USER_DIRS)
    {
        std::vector<UserDir>& userDirs = searchItems[tabSearch->currentIndex()].vecUserDirs;
        QString dirName = index.data().toString();
        std::vector<UserDir>::iterator iter;
        for (iter = userDirs.begin(); iter != userDirs.end(); ++iter)
        {
            if(iter->dirPath == dirName && iter->vecFiles.size() > 0)
            {
                iter->bExpanded = true;
                if (!iter->bFilled)
                {
                    QModelIndex real_index = filterModel->mapToSource(index);
                    std::vector<QED2KSearchResultEntry>::const_iterator file_iter;
                    int sub_row = 0;
                    for (file_iter = iter->vecFiles.begin(); file_iter != iter->vecFiles.end(); ++file_iter)
                    {
                        fillFileValues(sub_row, *file_iter, real_index);
                        sub_row++;
                    }
                    iter->bFilled = true;
                }
                break;
            }
        }
    }
    else if (searchItems[tabSearch->currentIndex()].resultType == RT_FOLDERS)
    {
        QModelIndex real_index = filterModel->mapToSource(index);
        QString hash = model->data(model->index(real_index.row(), SWDelegate::SW_ID)).toString();

        std::vector<QED2KSearchResultEntry> const& vRes = searchItems[tabSearch->currentIndex()].vecResults;
        std::vector<QED2KSearchResultEntry>::const_iterator it;
        std::vector<UserDir>& userDirs = searchItems[tabSearch->currentIndex()].vecUserDirs;
        std::vector<UserDir>::const_iterator dir_iter = userDirs.begin();
        for (it = vRes.begin(); it != vRes.end(); ++it, ++dir_iter)
            if (it->m_hFile == hash)
                break;

        if (dir_iter != userDirs.end())
        {
            if (!dir_iter->bFilled)
                model->item(real_index.row())->removeRows(0, 1);
        }
    }
}
