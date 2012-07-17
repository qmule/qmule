#include <QMenu>
#include <QAction>
#include <QDir>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QPainter>

#include "files_widget.h"
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
    model->setHeaderData(FW_NAME, Qt::Horizontal,           tr("File Name"));
    model->setHeaderData(FW_SIZE, Qt::Horizontal,           tr("File Size"));

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
    connect(treeFiles, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(itemClicked(QTreeWidgetItem*, int)));

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

    filesNotExchDir = new QAction(this);
    filesNotExchDir->setObjectName(QString::fromUtf8("filesNotExchDir"));
    filesNotExchDir->setText(tr("Don't exchange dir"));

    filesNotExchSubdir = new QAction(this);
    filesNotExchSubdir->setObjectName(QString::fromUtf8("filesNotExchSubdir"));
    filesNotExchSubdir->setText(tr("Don't exchange with subdirs"));

    filesMenu->addAction(filesExchDir);
    filesMenu->addAction(filesExchSubdir);
    filesMenu->addSeparator();
    filesMenu->addAction(filesNotExchDir);
    filesMenu->addAction(filesNotExchSubdir);

    connect(filesExchDir,  SIGNAL(triggered()), this, SLOT(exchangeDir()));
    connect(filesExchSubdir,  SIGNAL(triggered()), this, SLOT(exchangeSubdir()));
    connect(filesNotExchDir,  SIGNAL(triggered()), this, SLOT(notExchangeDir()));
    connect(filesNotExchSubdir,  SIGNAL(triggered()), this, SLOT(notExchangeSubdir()));

    tabWidget->hide();
}

files_widget::~files_widget()
{

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
    QList<QTreeWidgetItem*> children = item->takeChildren();
    children.erase(children.begin(), children.end());
}

void files_widget::itemClicked(QTreeWidgetItem* item, int column)
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
        listItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        if (isDir && !dirRules.contains(dirPath))
        {
            if (fileRules.contains(dirPath + iter->fileName()))
                listItem->setCheckState(Qt::Checked);
            else
                listItem->setCheckState(Qt::Unchecked);
        }
        else
        {
            if (dirRules[dirPath].contains(iter->fileName()))
                listItem->setCheckState(Qt::Unchecked);
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
                Session::instance()->get_ed2k_session()->delegate()->share_file(filePath.toStdString());
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
                Session::instance()->get_ed2k_session()->delegate()->unshare_file(filePath.toStdString());
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
                files.removeOne(fileName);
        }
        else
        {
            if (!files.contains(fileName))
                files.push_back(fileName);
        }
        shareDir(dirPath, true);
    }
}

void files_widget::displayTreeMenu(const QPoint&)
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();

    if (!curItem)
        return;

    if (!isDirTreeItem(curItem))
        return;

    filesMenu->exec(QCursor::pos());
}

void files_widget::generateSharedTree()
{
    QList<QTreeWidgetItem*> children = sharedDirs->takeChildren();
    children.erase(children.begin(), children.end());

    if (!dirRules.size())
        return;

    shared_entry::iterator iter;
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
    QString strPath = getDirPath(curItem);

    if (!dirRules.contains(strPath))
    {
        QList<QString> filesList;
        dirRules.insert(strPath, filesList);
        shareDir(strPath, true);

        setExchangeStatus(curItem, true);
        generateSharedTree();
        itemClicked(curItem, 0);
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
    QString strPath = getDirPath(curItem);

    QDir dir(strPath);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);
    if (!dirRules.contains(strPath))
    {
        QList<QString> filesList;
        dirRules.insert(strPath, filesList);
        shareDir(strPath, true);

        setChildExchangeStatus(curItem, true);
        itemClicked(curItem, 0);
        while (curItem->parent() != allDirs)
        {
            curItem = curItem->parent();
            curItem->setFont(0, boldFont);
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

            if (strPath.lastIndexOf("/") != (strPath.length() - 1))
                strPath += "/";

            QDir dir(strPath);
            QFileInfoList subList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);
            if (!dirRules.contains(strPath))
            {
                dirRules.insert(strPath, filesList);
                shareDir(strPath, true);
            }

            exchangeSubdir(subList);
        }
    }
}

void files_widget::notExchangeDir()
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();
    QString strPath = getDirPath(curItem);

    shareDir(strPath, false);
    dirRules.erase(dirRules.find(strPath));

    setExchangeStatus(curItem, false);
    checkExchangeParentStatus(curItem);

    itemClicked(curItem, 0);

    generateSharedTree();
}

void files_widget::notExchangeSubdir()
{
    QTreeWidgetItem* curItem = treeFiles->currentItem();
    QString strPath = getDirPath(curItem);

    shared_entry::iterator iter = dirRules.begin();
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

    setChildExchangeStatus(curItem, false);
    checkExchangeParentStatus(curItem);
    itemClicked(curItem, 0);

    generateSharedTree();
}

void files_widget::checkExchangeParentStatus(QTreeWidgetItem* curItem)
{
    QString strPath = getDirPath(curItem);
    if (partOfSharedPath(strPath))
        curItem->setFont(0, boldFont);
    else
        curItem->setFont(0, usualFont);

    while (curItem->parent() != allDirs)
    {
        curItem = curItem->parent();
        strPath = getDirPath(curItem);
        if (dirRules.contains(strPath))
            break;
        if (!partOfSharedPath(strPath))
            curItem->setFont(0, usualFont);
    }
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
/*    Session::instance()->get_ed2k_session()->delegate()->begin_share_transaction();

    QString curParentDir = dirRules.begin().key() + "!";
    QString basePath;
    shared_entry::iterator iter;
    QList<QString>::iterator filesIter;
    std::deque<std::string> files;

    for (iter = dirRules.begin(); iter != dirRules.end(); ++iter)
    {
        QString dirPath = iter.key();
        if (!dirPath.startsWith(curParentDir))
        {
            curParentDir = dirPath;
            basePath = dirPath;
            if (basePath.lastIndexOf('/') == (basePath.length() - 1))
                basePath = basePath.left(basePath.length() - 1);
            basePath = basePath.left(basePath.lastIndexOf('/'));
        }
        files.clear();
        QList<QString>& dirFiles = dirRules.value(dirPath);
        for (filesIter = dirFiles.begin(); filesIter != dirFiles.end(); ++filesIter)
            files.push_back(filesIter->toStdString());

        Session::instance()->get_ed2k_session()->delegate()->share_dir(basePath.toStdString(), dirPath.toStdString(), files);
    }

    for (filesIter = fileRules.begin(); filesIter != fileRules.end(); ++filesIter)
        Session::instance()->get_ed2k_session()->delegate()->share_file(filesIter->toStdString());

    Session::instance()->get_ed2k_session()->delegate()->end_share_transaction();*/
}

void files_widget::shareDir(QString dirPath, bool bShare)
{
    shared_entry::iterator iter;
    QList<QString>::const_iterator filesIter;
    std::deque<std::string> files;

    QString basePath;
    for (iter = dirRules.begin(); iter != dirRules.end(); ++iter)
    {
        if (dirPath.startsWith(iter.key()))
        {
            basePath = iter.key();
            if (basePath.lastIndexOf('/') == (basePath.length() - 1))
                basePath = basePath.left(basePath.length() - 1);
            basePath = basePath.left(basePath.lastIndexOf('/'));
            break;
        }
    }

    if (!basePath.length())
    {
        basePath = dirPath;
        if (basePath.lastIndexOf('/') == (basePath.length() - 1))
            basePath = basePath.left(basePath.length() - 1);
        basePath = basePath.left(basePath.lastIndexOf('/'));
    }

    const QList<QString> dirFiles = dirRules.value(dirPath);
    if (dirPath.lastIndexOf('/') == (dirPath.length() - 1))
        dirPath = dirPath.left(dirPath.length() - 1);

    for (filesIter = dirFiles.begin(); filesIter != dirFiles.end(); ++filesIter)
        files.push_back(filesIter->toStdString());

    if (bShare)
        Session::instance()->get_ed2k_session()->delegate()->share_dir(basePath.toStdString(), dirPath.toStdString(), files);
    else
        Session::instance()->get_ed2k_session()->delegate()->unshare_dir(basePath.toStdString(), dirPath.toStdString(), files);
}

void files_widget::setExchangeStatus(QTreeWidgetItem* item, bool status)
{
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
    shared_entry::iterator iter;
    for (iter = dirRules.begin(); iter != dirRules.end(); ++iter)
        if (iter.key().startsWith(path))
            return true;

    QList<QString>::iterator filesIter;
    for (filesIter = fileRules.begin(); filesIter != fileRules.end(); ++filesIter)
        if (filesIter->startsWith(path))
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
