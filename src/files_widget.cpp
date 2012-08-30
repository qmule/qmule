#include <QMenu>
#include <QAction>
#include <QDir>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QPainter>
#include <QClipboard>

#include "files_widget.h"
#include "preferences.h"
#include "libed2k/types.hpp"
#include "transport/session.h"

files_widget::files_widget(QWidget *parent)
    : QWidget(parent), bProcessFiles(false)
{
    setupUi(this);

    labelIcon->setPixmap(QIcon(":/emule/files/SharedFilesList.ico").pixmap(16, 16));
    QIcon overlay(":/emule/files/SharedFolderOvl.png");
    emuleFolder = provider.icon(QFileIconProvider::Folder);

    QSize size = emuleFolder.availableSizes()[0];
    QImage img(size, QImage::Format_ARGB32);
    QPainter painter;

    painter.begin(&img);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    painter.drawPixmap(0, 0, emuleFolder.pixmap(size));
    painter.drawPixmap(0, 0, overlay.pixmap(size));

    painter.end();

    emuleFolder = QIcon(QPixmap::fromImage(img));

    QList<int> sizes;
    sizes.append(100);
    sizes.append(500);
    splitter->setSizes(sizes);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    allFiles = new QTreeWidgetItem(treeFiles);
    allFiles->setText(0, tr("All exchange files"));
    allFiles->setIcon(0, QIcon(":/emule/files/all.ico"));
    allFiles->setExpanded(true);

    sharedDirs = new QTreeWidgetItem(allFiles);
    sharedDirs->setText(0, tr("Exchange folders"));
    sharedDirs->setIcon(0, provider.icon(QFileIconProvider::Folder));
    sharedDirs->setExpanded(true);

    allDirs = new QTreeWidgetItem(treeFiles);
    allDirs->setText(0, tr("All dirs"));
    allDirs->setIcon(0, QIcon(":/emule/files/HardDisk.ico"));
    allDirs->setExpanded(true);

    Preferences pref;
    dirRules = pref.loadSharedDirs();
    fileRules = pref.loadSharedFiles();
    saveDirPath = pref.getSavePath();
    addLastSlash(saveDirPath);
    generateSharedTree();

    QFileInfoList drivesList = QDir::drives();

    usualFont = treeFiles->font();
    boldFont = treeFiles->font();
    boldFont.setBold(true);
    QFileInfoList::const_iterator iter;
    for (iter = drivesList.begin(); iter != drivesList.end(); ++iter)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(allDirs);
        QString path = iter->absolutePath();
        if (partOfSharedPath(path))
            item->setFont(0, boldFont);
        item->setText(0, path.replace(":/", ":"));
        item->setIcon(0, provider.icon(QFileIconProvider::Folder));
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }

    model.reset(new QStandardItemModel(0, FW_COLUMNS_NUM));
    model->setHeaderData(FW_NAME, Qt::Horizontal, tr("File Name"));
    model->setHeaderData(FW_SIZE, Qt::Horizontal, tr("File Size"));
    model->setHeaderData(FW_ID, Qt::Horizontal,   tr("File ID"));

    filterModel.reset(new QSortFilterProxyModel());
    filterModel.data()->setDynamicSortFilter(true);
    filterModel.data()->setSourceModel(model.data());
    filterModel.data()->setFilterKeyColumn(FW_NAME);
    filterModel.data()->setFilterRole(Qt::DisplayRole);
    filterModel.data()->setSortCaseSensitivity(Qt::CaseInsensitive);

    tableFiles->setModel(filterModel.data());
    tableFiles->setColumnWidth(0, 250);
    tableFiles->header()->setSortIndicator(FW_NAME, Qt::AscendingOrder);

    connect(treeFiles, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
    connect(treeFiles, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsed(QTreeWidgetItem*)));
    connect(treeFiles, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

    treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeFiles, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayTreeMenu(const QPoint&)));

    connect(model.data(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(tableItemChanged(QStandardItem*)));

    connect(btnApply, SIGNAL(clicked()), this, SLOT(applyChanges()));

    filesMenu = new QMenu(this);
    filesMenu->setObjectName(QString::fromUtf8("filesMenu"));
    filesMenu->setTitle(tr("Exchange files"));

    filesExchDir = new QAction(this);
    filesExchDir->setObjectName(QString::fromUtf8("filesExchDir"));
    filesExchDir->setText(tr("Exchange dir"));

    filesExchSubdir = new QAction(this);
    filesExchSubdir->setObjectName(QString::fromUtf8("filesExchSubdir"));
    filesExchSubdir->setText(tr("Exchange with subdirs"));

    filesUnexchDir = new QAction(this);
    filesUnexchDir->setObjectName(QString::fromUtf8("filesUnexchDir"));
    filesUnexchDir->setText(tr("Don't exchange dir"));

    filesUnexchSubdir = new QAction(this);
    filesUnexchSubdir->setObjectName(QString::fromUtf8("filesUnexchSubdir"));
    filesUnexchSubdir->setText(tr("Don't exchange with subdirs"));

    filesMenu->addAction(filesExchDir);
    filesMenu->addAction(filesExchSubdir);
    filesMenu->addSeparator();
    filesMenu->addAction(filesUnexchDir);
    filesMenu->addAction(filesUnexchSubdir);

    connect(filesExchDir,  SIGNAL(triggered()), this, SLOT(exchangeDir()));
    connect(filesExchSubdir,  SIGNAL(triggered()), this, SLOT(exchangeSubdir()));
    connect(filesUnexchDir,  SIGNAL(triggered()), this, SLOT(unexchangeDir()));
    connect(filesUnexchSubdir,  SIGNAL(triggered()), this, SLOT(unxchangeSubdir()));

    connect(Session::instance()->get_ed2k_session(), SIGNAL(addedTransfer(Transfer)), this, SLOT(addedTransfer(Transfer)));
    connect(Session::instance()->get_ed2k_session(), SIGNAL(deletedTransfer(QString)), this, SLOT(deletedTransfer(QString)));

    connect(tableFiles->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(selectedFileChanged(const QItemSelection&, const QItemSelection&)));

    connect(checkForum, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));
    connect(checkSize, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));
    connect(btnCopy, SIGNAL(clicked()), this, SLOT(putToClipboard()));
}

files_widget::~files_widget()
{
    Preferences pref;
    pref.saveSharedDirs(dirRules);
    pref.saveSharedFiles(fileRules);
}

void files_widget::itemExpanded(QTreeWidgetItem* item)
{
    if (!isDirTreeItem(item))
        return;

    QDir dir(getDirPath(item));
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);

    QFileInfoList::const_iterator iter;
    QIcon dirIcon(":/emule/common/FolderOpen.ico");
    for (iter = fileList.begin(); iter != fileList.end(); ++iter)
    {
        if (iter->isDir())
        {
            QTreeWidgetItem* newItem = new QTreeWidgetItem(item);
            newItem->setText(0, iter->fileName());

            QString strPath = getDirPath(newItem);
            if (dirRules.contains(strPath))
                setExchangeStatus(newItem, true);
            else
            {
                newItem->setIcon(0, provider.icon(QFileIconProvider::Folder));
                if (item->font(0) == boldFont)
                    if (partOfSharedPath(strPath))
                        newItem->setFont(0, boldFont); 
            }

            if (strPath == saveDirPath)
                setExchangeStatus(newItem, true);

            QDir dirInfo(iter->absoluteFilePath());
            QFileInfoList dirList = dirInfo.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs);
            if (dirList.size() > 0)
                newItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
            else
                newItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        }
    }
}

void files_widget::itemCollapsed(QTreeWidgetItem* item)
{
    if (isDirTreeItem(item))
    {
        QList<QTreeWidgetItem*> children = item->takeChildren();
        children.erase(children.begin(), children.end());
    }
}

void files_widget::currentItemChanged(QTreeWidgetItem* item, QTreeWidgetItem* olditem)
{
    for (int ii = model->rowCount()-1; ii >= 0; --ii)
        model->removeRow(ii);

    if (item == allFiles)
    {
        std::vector<Transfer> transfers = Session::instance()->get_ed2k_session()->getTransfers();
        std::vector<Transfer>::const_iterator iter;
        for (iter = transfers.begin(); iter != transfers.end(); ++iter)
        {
            model->insertRow(0);
            model->setData(model->index(0, FW_NAME), iter->name());
            model->setData(model->index(0, FW_SIZE), misc::friendlyUnit(iter->actual_size()));
            model->setData(model->index(0, FW_ID),   iter->hash());

            QFileInfo info(iter->save_path() + "/" + iter->name());
            QStandardItem* listItem = model->item(0, FW_NAME);
            listItem->setIcon(provider.icon(info));
        }

        return;
    }

    bool isDir = isDirTreeItem(item);
    if (!isDir && !isSharedDirTreeItem(item))
        return;

    bProcessFiles = true;

    QString dirPath;
    if (isDir)
        dirPath = getDirPath(item);
    else
        dirPath = item->data(0, Qt::UserRole).toString();

    QDir dir(dirPath);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files);

    QFileInfoList::const_iterator iter;
    int row = 0;
    for (iter = fileList.begin(); iter != fileList.end(); ++iter)
    {
        model->insertRow(row);
        model->setData(model->index(row, FW_NAME), iter->fileName());
        model->setData(model->index(row, FW_SIZE), misc::friendlyUnit(iter->size()));
        QStandardItem* listItem = model->item(row, FW_NAME);
        listItem->setIcon(provider.icon(*iter));

        if (dirPath == saveDirPath)
        {
            listItem->setCheckState(Qt::PartiallyChecked);
            listItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            model->setData(model->index(row, FW_ID), getHashByPath(dirPath + iter->fileName()));
            continue;
        }

        listItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        if (isDir && !dirRules.contains(dirPath))
        {
            if (fileRules.contains(dirPath + iter->fileName()))
            {
                listItem->setCheckState(Qt::Checked);
                model->setData(model->index(row, FW_ID), getHashByPath(dirPath + iter->fileName()));
            }
            else
                listItem->setCheckState(Qt::Unchecked);
        }
        else
        {
            if (dirRules[dirPath].contains(iter->fileName()))
            {
                listItem->setCheckState(Qt::Unchecked);
                model->setData(model->index(row, FW_ID), getHashByPath(dirPath + iter->fileName()));
            }
            else
                listItem->setCheckState(Qt::Checked);
        }

    }

    bProcessFiles = false;
}

void files_widget::tableItemChanged(QStandardItem* item)
{
    if (bProcessFiles)
        return;

    if (item->column())
        return;

    QTreeWidgetItem* curItem = treeFiles->currentItem();
    if (!curItem)
        return;

    bool isDir = isDirTreeItem(curItem);
    if (!isDir && !isSharedDirTreeItem(curItem))
        return;

    QString dirPath;
    if (isDir)
        dirPath = getDirPath(curItem);
    else
        dirPath = curItem->data(0, Qt::UserRole).toString();
    QString fileName = item->text();

    if (isDir && !dirRules.contains(dirPath))
    {
        QString filePath = dirPath + fileName;
        if (item->checkState() == Qt::Checked)
        {
            if (!fileRules.contains(filePath))
            {
                fileRules.push_back(filePath);
                Session::instance()->get_ed2k_session()->delegate()->share_file(filePath.toUtf8().constData());
                curItem->setFont(0, boldFont);
                while (curItem->parent() != allDirs)
                {
                    curItem = curItem->parent();
                    curItem->setFont(0, boldFont);
                }
            }
        }
        else
        {
            if (fileRules.contains(filePath))
            {
                fileRules.removeOne(filePath);
                removeTransferPath(filePath);
                Session::instance()->get_ed2k_session()->delegate()->unshare_file(filePath.toUtf8().constData());
                checkExchangeParentStatus(curItem);
            }
        }
    }
    else
    {
        QList<QString>& files = dirRules[dirPath];
        if (item->checkState() == Qt::Checked)
        {
            if (files.contains(fileName))
            {
                files.removeOne(fileName);
                QString filePath = dirPath + fileName;
                Session::instance()->get_ed2k_session()->delegate()->share_file(filePath.toUtf8().constData());
            }
        }
        else
        {
            if (!files.contains(fileName))
            {
                files.push_back(fileName);
                QString filePath = dirPath + fileName;
                removeTransferPath(filePath);
                Session::instance()->get_ed2k_session()->delegate()->unshare_file(filePath.toUtf8().constData());
            }
        }        
    }
}

void files_widget::displayTreeMenu(const QPoint&)
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();

    if (!curItem)
        return;

    if (isDirTreeItem(curItem))
    {
        filesExchDir->setEnabled(!isExchangeDir(curItem));
        filesExchSubdir->setEnabled(true);
        filesUnexchDir->setEnabled(!isExchangeDir(curItem));
        filesUnexchSubdir->setEnabled(true);
        filesMenu->exec(QCursor::pos());
    }
    if (isSharedDirTreeItem(curItem))
    {
        filesExchDir->setEnabled(false);
        filesExchSubdir->setEnabled(false);
        filesUnexchDir->setEnabled(true);
        filesUnexchSubdir->setEnabled(true);
        filesMenu->exec(QCursor::pos());
    }    
}

void files_widget::generateSharedTree()
{
    QList<QTreeWidgetItem*> children = sharedDirs->takeChildren();
    children.erase(children.begin(), children.end());

    if (!dirRules.size())
        return;

    shared_map::iterator iter;
    QVector<QString> stackDirs;
    QString curParentDir = dirRules.begin().key() + "!";
    QTreeWidgetItem* curParentNode = sharedDirs;

    for (iter = dirRules.begin(); iter != dirRules.end(); ++iter)
    {
        QString dirPath = iter.key();
        QDir dir(dirPath);
        if (!dir.exists())
            continue;

        while (!dirPath.startsWith(curParentDir) && !stackDirs.empty())
        {
            stackDirs.pop_back();
            if (!stackDirs.empty())
                curParentDir = stackDirs.last();            
            curParentNode = curParentNode->parent();
        }

        QString nodeName = dir.dirName();
        if (curParentNode == sharedDirs && dirPath.indexOf(':') >= 0)
            nodeName += " (" + dirPath.left(dirPath.indexOf(':') + 1) + ")";

        QTreeWidgetItem* newItem = new QTreeWidgetItem(curParentNode);
        newItem->setText(0, nodeName);
        newItem->setIcon(0, provider.icon(QFileIconProvider::Folder));
        newItem->setData(0, Qt::UserRole, dirPath);

        curParentNode = newItem;
        curParentDir = dirPath;
        stackDirs.push_back(dirPath);
    }
}

void files_widget::exchangeDir()
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();
    if (!curItem)
        return;

    QString strPath = getDirPath(curItem);

    if (strPath == saveDirPath)
        return;

    if (!dirRules.contains(strPath))
    {
        QList<QString> filesList;        
        checkBaseAdd(strPath);
        dirRules.insert(strPath, filesList);
        shareDir(strPath, true);

        setExchangeStatus(curItem, true);
        generateSharedTree();
        currentItemChanged(curItem, NULL);
        while (curItem->parent() != allDirs)
        {
            curItem = curItem->parent();
            curItem->setFont(0, boldFont);
        }        
    }
}

void files_widget::exchangeSubdir()
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();
    if (!curItem)
        return;

    QString strPath = getDirPath(curItem);
    
    QDir dir(strPath);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);

    setChildExchangeStatus(curItem, true);
    if (!dirRules.contains(strPath))
    {
        if (strPath != saveDirPath)
        {
            QList<QString> filesList;
            checkBaseAdd(strPath);
            dirRules.insert(strPath, filesList);
            shareDir(strPath, true);

            currentItemChanged(curItem, NULL);
        }

        bool setParentState = true;
        if (strPath == saveDirPath)
        {
            setParentState = false;
            QFileInfoList::const_iterator iter;
            for (iter = fileList.begin(); iter != fileList.end(); ++iter)
            {
                if (iter->isDir())
                {
                    setParentState = true;
                    break;
                }
            }
        }

        if (setParentState)
        {
            while (curItem->parent() != allDirs)
            {
                curItem = curItem->parent();
                curItem->setFont(0, boldFont);
            }
        }
    }
    exchangeSubdir(fileList);    

    generateSharedTree();
}

void files_widget::exchangeSubdir(QFileInfoList& fileList)
{
    QFileInfoList::const_iterator iter;
    QList<QString> filesList;
    for (iter = fileList.begin(); iter != fileList.end(); ++iter)
    {
        if (iter->isDir())
        {
            QString strPath = iter->absoluteFilePath();
            addLastSlash(strPath);

            QDir dir(strPath);
            QFileInfoList subList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);
            if (!dirRules.contains(strPath) && strPath != saveDirPath)
            {
                dirRules.insert(strPath, filesList);
                shareDir(strPath, true);
            }

            exchangeSubdir(subList);
        }
    }
}

void files_widget::unexchangeDir()
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();
    if (!curItem)
        return;
 
    QString strPath;
    if (isDirTreeItem(curItem))
        strPath = getDirPath(curItem);
    if (isSharedDirTreeItem(curItem))
        strPath = curItem->data(0, Qt::UserRole).toString();

    if (strPath == saveDirPath)
        return;

    checkBaseRemove(strPath);
    shareDir(strPath, false);
    dirRules.erase(dirRules.find(strPath));

    if (isDirTreeItem(curItem))
    {
        setExchangeStatus(curItem, false);
        checkExchangeParentStatus(curItem);
        currentItemChanged(curItem, NULL);
    }
    else if (isSharedDirTreeItem(curItem))
    {
        applyUnexchangeStatus(strPath, false);
    }

    generateSharedTree();
}

void files_widget::unxchangeSubdir()
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();
    if (!curItem)
        return;

    QString strPath;
    if (isDirTreeItem(curItem))
        strPath = getDirPath(curItem);
    if (isSharedDirTreeItem(curItem))
        strPath = curItem->data(0, Qt::UserRole).toString();

    shared_map::iterator iter = dirRules.begin();
    for (iter = dirRules.begin(); iter != dirRules.end(); ++iter)
        if (iter.key().startsWith(strPath))
            shareDir(iter.key(), false);

    iter = dirRules.begin();
    while (iter != dirRules.end())
    {
        if (iter.key().startsWith(strPath))
            iter = dirRules.erase(iter);
        else
            ++iter;
    }

    if (isDirTreeItem(curItem))
    {
        setChildExchangeStatus(curItem, false);
        checkExchangeParentStatus(curItem);
        currentItemChanged(curItem, NULL);
    }
    else if (isSharedDirTreeItem(curItem))
    {
        applyUnexchangeStatus(strPath, true);
    }

    generateSharedTree();
}

void files_widget::checkExchangeParentStatus(QTreeWidgetItem* curItem)
{
    do
    {
        QString strPath = getDirPath(curItem);
        if (!strPath.length())
            return;
        if (dirRules.contains(strPath))
            return;

        if (partOfSharedPath(strPath))
            curItem->setFont(0, boldFont);
        else
            curItem->setFont(0, usualFont);

        curItem = curItem->parent();
    }
    while (curItem != allDirs);
}

bool files_widget::isDirTreeItem(QTreeWidgetItem* item)
{
    if (item == allDirs || item == allFiles)
        return false;

    QTreeWidgetItem* itemIter = item;
    while (itemIter->parent() != allDirs && itemIter->parent() != allFiles)
        itemIter = itemIter->parent();

    if (itemIter->parent() == allFiles)
        return false;

    return true;
}

bool files_widget::isExchangeDir(QTreeWidgetItem* item)
{
    return isDirTreeItem(item) ? (getDirPath(item) == saveDirPath) : false;
}

bool files_widget::isSharedDirTreeItem(QTreeWidgetItem* item)
{
    if (item == allDirs || item == allFiles)
        return false;

    QTreeWidgetItem* itemIter = item;
    while (itemIter->parent() != allDirs && itemIter->parent() != allFiles && itemIter->parent() != sharedDirs)
        itemIter = itemIter->parent();

    if (itemIter->parent() == sharedDirs)
        return true;

    return false;
}

QString files_widget::getDirPath(QTreeWidgetItem* item)
{
    if (!isDirTreeItem(item))
        return "";

    QString path = item->text(0) + "/";

    QTreeWidgetItem* itemIter = item;
    while (itemIter->parent() != allDirs)
    {
        itemIter = itemIter->parent();
        path = itemIter->text(0) + "/" + path;
    }

    path.replace("//","/");
    return path;
}

void files_widget::applyChanges()
{
    Preferences pref;
    pref.saveSharedDirs(dirRules);
    pref.saveSharedFiles(fileRules);
}

void files_widget::shareDir(QString dirPath, bool bShare)
{
    shared_map::iterator iter;
    QList<QString>::const_iterator filesIter;
    std::deque<std::string> files;

    QString basePath;
    for (iter = dirRules.begin(); iter != dirRules.end(); ++iter)
    {
        if (dirPath.startsWith(iter.key()))
        {
            basePath = iter.key();
            removeLastSlash(basePath);
            basePath = basePath.left(basePath.lastIndexOf('/'));
            break;
        }
    }

    if (!basePath.length())
    {
        basePath = dirPath;
        removeLastSlash(basePath);
        basePath = basePath.left(basePath.lastIndexOf('/'));
    }

    const QList<QString> dirFiles = dirRules.value(dirPath);
    removeLastSlash(dirPath);

    for (filesIter = dirFiles.begin(); filesIter != dirFiles.end(); ++filesIter)
        files.push_back(filesIter->toStdString());

    Session::instance()->get_ed2k_session()->delegate()->share_dir(
        basePath.toUtf8().constData(), dirPath.toUtf8().constData(), files, !bShare);
}

void files_widget::checkBaseAdd(QString dirPath)
{
    bool bNewBase = false;
    QString curBase;
    QString newBase = dirPath;
    removeLastSlash(newBase);
    newBase = newBase.left(newBase.lastIndexOf('/'));
    std::deque<std::string> files;

    for (shared_map::iterator iter = dirRules.begin(); iter != dirRules.end(); ++iter)
    {
        if (iter.key().startsWith(dirPath))
        {
            curBase = iter.key();
            removeLastSlash(curBase);
            curBase = curBase.left(curBase.lastIndexOf('/'));
            bNewBase = true;
        }
        if (bNewBase)
        {
            if (iter.key().startsWith(dirPath))
            {
                QString curDirPath = iter.key();
                const QList<QString> dirFiles = dirRules.value(curDirPath);
                files.clear();
                for (QList<QString>::const_iterator filesIter = dirFiles.begin(); filesIter != dirFiles.end(); ++filesIter)
                    files.push_back(filesIter->toStdString());
                removeLastSlash(curDirPath);
                Session::instance()->get_ed2k_session()->delegate()->unshare_dir(curBase.toUtf8().constData(), curDirPath.toUtf8().constData(), files);
                Session::instance()->get_ed2k_session()->delegate()->share_dir(newBase.toUtf8().constData(), curDirPath.toUtf8().constData(), files);
            }
            else
                break;
        }
    }
}

void files_widget::checkBaseRemove(QString dirPath)
{
    bool bNewBase = false;
    QString oldBase = dirPath;
    removeLastSlash(oldBase);
    oldBase = oldBase.left(oldBase.lastIndexOf('/'));
    QString newBase;
    std::deque<std::string> files;

    for (shared_map::iterator iter = dirRules.begin(); iter != dirRules.end(); ++iter)
    {        
        if (dirPath.startsWith(iter.key()))
        {
             if (dirPath != iter.key())
             {
                 return;
             }
             bNewBase = true;
             continue;
        }
        if (bNewBase)
        {
            if (!newBase.length())
            {
                newBase = iter.key();
                removeLastSlash(newBase);
                newBase = newBase.left(newBase.lastIndexOf('/'));
            }
            if (!iter.key().startsWith(newBase))
                return;
            QString curDirPath = iter.key();
            const QList<QString> dirFiles = dirRules.value(curDirPath);
            files.clear();
            for (QList<QString>::const_iterator filesIter = dirFiles.begin(); filesIter != dirFiles.end(); ++filesIter)
                files.push_back(filesIter->toStdString());
            removeLastSlash(curDirPath);
            Session::instance()->get_ed2k_session()->delegate()->unshare_dir(oldBase.toUtf8().constData(), curDirPath.toUtf8().constData(), files);
            Session::instance()->get_ed2k_session()->delegate()->share_dir(newBase.toUtf8().constData(), curDirPath.toUtf8().constData(), files);
        }
    }
}

void files_widget::setExchangeStatus(QTreeWidgetItem* item, bool status)
{
    if (isExchangeDir(item) && !status)
        return;

    if (status)
    {
        item->setFont(0, boldFont);
        item->setIcon(0, emuleFolder);
    }
    else
    {
        item->setFont(0, usualFont);
        item->setIcon(0, provider.icon(QFileIconProvider::Folder));
    }
}

bool files_widget::partOfSharedPath(QString path)
{
    shared_map::iterator iter;
    for (iter = dirRules.begin(); iter != dirRules.end(); ++iter)
        if (iter.key().startsWith(path))
            return true;

    QList<QString>::iterator filesIter;
    for (filesIter = fileRules.begin(); filesIter != fileRules.end(); ++filesIter)
        if (filesIter->startsWith(path))
            return true;

    if (saveDirPath.startsWith(path))
        return true;

    return false;
}

void files_widget::setChildExchangeStatus(QTreeWidgetItem* item, bool status)
{
    setExchangeStatus(item, status);

    int childCnt = item->childCount();
    for (int ii = 0; ii < childCnt; ii++)
    {
        setChildExchangeStatus(item->child(ii), status);
    }
}

void files_widget::applyUnexchangeStatus(QString strPath, bool recursive)
{
    QTreeWidgetItem* curItem = NULL;

    if (findTreeItem(curItem, strPath))
    {
        if (recursive)
            setChildExchangeStatus(curItem, false);
        else
            setExchangeStatus(curItem, false);
    }
    checkExchangeParentStatus(curItem);
}

bool files_widget::findTreeItem(QTreeWidgetItem*& item, QString strPath)
{
    item = allDirs;
    QString begin = strPath.left(strPath.indexOf('/'));
    if (!begin.length())
        begin = "/";
    QString end = strPath.right(strPath.length() - strPath.indexOf('/') - 1);
    bool deeper = true;
    while (deeper)
    {
        deeper = false;
        int childCnt = item->childCount();
        for (int ii = 0; ii < childCnt; ii++)
        {
            if (item->child(ii)->data(0, Qt::DisplayRole).toString() == begin)
            {
                item = item->child(ii);
                if (end.indexOf('/') >= 0)
                {
                    begin = end.left(end.indexOf('/'));
                    end = end.right(end.length() - end.indexOf('/') - 1);
                }
                else
                {
                    begin = "";
                    deeper = false;
                    break;
                }
                deeper = true;
                break;
            }
        }
    }

    if (begin.size())
        return false;

    return true;
}

void files_widget::removeLastSlash(QString& dirPath)
{
    if (dirPath.lastIndexOf('/') == (dirPath.length() - 1))
        dirPath = dirPath.left(dirPath.length() - 1);
}

void files_widget::addLastSlash(QString& dirPath)
{
    if (dirPath.lastIndexOf('/') != (dirPath.length() - 1))
        dirPath += "/";
}

void files_widget::addedTransfer(Transfer transfer)
{
    QString filePath = transfer.absolute_files_path().at(0);
    if (!transferPath.contains(transfer.hash()))
        transferPath.insert(transfer.hash(), filePath);
    
    QString collectionPath(Session::instance()->get_ed2k_session()->delegate()->settings().m_collections_directory.c_str());
    addLastSlash(collectionPath);

    QFileInfo file(filePath);
    QTreeWidgetItem* curItem;
    if (file.exists())
    {
        QString dirPath = file.absolutePath();
        addLastSlash(dirPath);
        if (dirRules.contains(dirPath))
        {
            if (dirRules[dirPath].contains(file.fileName()))
                dirRules[dirPath].removeOne(file.fileName());
        }
        else if (dirPath != saveDirPath && !dirPath.startsWith(collectionPath) && !fileRules.contains(filePath))
        {
            fileRules.push_back(filePath);
            findTreeItem(curItem, saveDirPath);
            checkExchangeParentStatus(curItem);
        }
    }

    curItem = treeFiles->currentItem();
    if (curItem)
        currentItemChanged(curItem, NULL);
}

void files_widget::deletedTransfer(QString hash)
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();

    if (!curItem || curItem == allDirs)
        return;

    if (curItem == allFiles)
    {
        currentItemChanged(curItem, NULL);
        return;
    }

    if (!transferPath.contains(hash))
    {
        currentItemChanged(curItem, NULL);
        return;
    }

    QString filePath = transferPath.value(hash);
    transferPath.remove(hash);

    if (fileRules.contains(filePath))
    {
        fileRules.removeOne(filePath);
        applyUnexchangeStatus(filePath, false);
    }
    else
    {
        QFileInfo file(filePath);
        if (file.exists())
        {
            QString dirPath = file.absolutePath();
            addLastSlash(dirPath);
            if (dirRules.contains(dirPath))
            {
                if (!dirRules[dirPath].contains(file.fileName()))
                    dirRules[dirPath].append(file.fileName());
            }
        }
    }

    currentItemChanged(curItem, NULL);
}

void files_widget::optionsChanged()
{
    Preferences pref;
    QString newDirPath = pref.getSavePath();
    addLastSlash(newDirPath);
    if (newDirPath == saveDirPath)
        return;

    // Unshare old Incoming directory
    std::deque<std::string> files;
    QString dirPath = saveDirPath;
    removeLastSlash(dirPath);

    QString basePath = saveDirPath;
    removeLastSlash(basePath);
    basePath = basePath.left(basePath.lastIndexOf('/'));

    Session::instance()->get_ed2k_session()->delegate()->unshare_dir(basePath.toUtf8().constData(), dirPath.toUtf8().constData(), files);

    // Share new Incoming directory
    QTreeWidgetItem* curItem = NULL;
    if (dirRules.contains(newDirPath))
    {
        checkBaseRemove(newDirPath);
        dirRules.remove(newDirPath);
        generateSharedTree();
        shareDir(newDirPath, false);
    }
    dirPath = newDirPath;
    removeLastSlash(dirPath);

    basePath = newDirPath;
    removeLastSlash(basePath);
    basePath = basePath.left(basePath.lastIndexOf('/'));

    Session::instance()->get_ed2k_session()->delegate()->share_dir(basePath.toUtf8().constData(), dirPath.toUtf8().constData(), files);
        
    QList<QString>::iterator filesIter = fileRules.begin();
    while (filesIter != fileRules.end())
    {
        if (filesIter->startsWith(newDirPath))
            filesIter = fileRules.erase(filesIter);
        else
            ++filesIter;
    }

    dirPath = saveDirPath;
    saveDirPath = newDirPath;

    applyUnexchangeStatus(dirPath, false);
    if (findTreeItem(curItem, saveDirPath))
    {
         setExchangeStatus(curItem, true);
    }
    checkExchangeParentStatus(curItem);
}

void files_widget::reshare()
{
    qDebug("reshare all files and dirs");
    std::deque<std::string> files;
    QString dirPath = saveDirPath;
    removeLastSlash(dirPath);

    QString basePath = saveDirPath;
    removeLastSlash(basePath);
    basePath = basePath.left(basePath.lastIndexOf('/'));

    Session::instance()->get_ed2k_session()->delegate()->share_dir(basePath.toUtf8().constData(), dirPath.toUtf8().constData(), files);

    QList<QString>::iterator filesIter = fileRules.begin();
    while (filesIter != fileRules.end())
    {
        QFileInfo file(*filesIter);
        if (file.exists() && file.isFile())
        {
            Session::instance()->get_ed2k_session()->delegate()->share_file(filesIter->toUtf8().constData());
            ++filesIter;
        }
        else
            filesIter = fileRules.erase(filesIter);
    }

    shared_map::iterator dirIter = dirRules.begin();
    while (dirIter != dirRules.end())
    {
        QFileInfo dir(dirIter.key());
        if (dir.exists() && dir.isDir())
            ++dirIter;
        else
            dirIter = dirRules.erase(dirIter);
    }

    for (dirIter = dirRules.begin(); dirIter != dirRules.end(); ++dirIter)
        shareDir(dirIter.key(), true);

    qDebug("finish resharing");
}

void files_widget::removeTransferPath(QString filePath)
{
    for (QMap<QString, QString>::iterator transferIter = transferPath.begin();
         transferIter != transferPath.end(); ++transferIter)
    {
        if (transferIter.value() == filePath)
        {
            transferPath.erase(transferIter);
            return;
        }
    }
}

void files_widget::selectedFileChanged(const QItemSelection& sel, const QItemSelection& unsel)
{
    createED2KLink();
}

void files_widget::checkChanged(int state)
{
    createED2KLink();
}

void files_widget::putToClipboard()
{
    QString text = editLink->toPlainText();
    if (text.length())
    {
        QClipboard *cb = QApplication::clipboard();
        cb->setText(text);
    }
}

void files_widget::createED2KLink()
{
    editLink->clear();
    checkForum->setDisabled(true);
    checkSize->setDisabled(true);
    btnCopy->setDisabled(true);

    QModelIndexList selected = tableFiles->selectionModel()->selectedRows();
    if (selected.empty())
        return;

    QTreeWidgetItem* curItem = treeFiles->currentItem();
    if (!curItem)
        return;

    int row = filterModel->mapToSource(selected.first()).row();

    QStandardItem* itemFile = model->item(row);
    if (!itemFile || (curItem != allFiles && itemFile->checkState() == Qt::Unchecked))
        return;
    QString fileName = itemFile->text();

    QString hash = model->data(model->index(row, FW_ID)).toString();
    if (!hash.length() || !transferPath.contains(hash))
        return;

    QFile file(transferPath.value(hash));
    if (!file.exists())
        return;

    checkForum->setEnabled(true);
    btnCopy->setEnabled(true);

    quint64 fileSize = file.size();
    QString size = QString::number(fileSize);

    QString link = "ed2k://|file|" + QString(QUrl::toPercentEncoding(fileName)) + "|" +
        size + "|" + hash + "|/";

    if (checkForum->checkState() == Qt::Checked)
    {
        link = "[u][b][url=" + link + "]" + fileName + "[/url][/b][/u]";
        checkSize->setEnabled(true);
        if (checkSize->checkState() == Qt::Checked)
            link += " " + misc::friendlyUnit(fileSize);
    }
    editLink->appendPlainText(link);
}

QString files_widget::getHashByPath(QString filePath)
{
    for (QMap<QString, QString>::iterator transferIter = transferPath.begin();
         transferIter != transferPath.end(); ++transferIter)
    {
        if (transferIter.value() == filePath)
        {
            return transferIter.key();
        }
    }
    return "";
}
