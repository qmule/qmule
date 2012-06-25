#include <QDir>
#include <QStandardItemModel>

#include "misc.h"
#include "files_widget.h"

files_widget::files_widget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    labelIcon->setPixmap(QIcon(":/emule/files/SharedFilesList.ico").pixmap(16, 16));

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

    allDirs = new QTreeWidgetItem(treeFiles);
    allDirs->setText(0, tr("All dirs"));
    allDirs->setIcon(0, QIcon(":/emule/files/HardDisk.ico"));
    allDirs->setExpanded(true);

    QFileInfoList drivesList = QDir::drives();

    QFileInfoList::const_iterator iter;
    for (iter = drivesList.begin(); iter != drivesList.end(); ++iter)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(allDirs);
        item->setText(0, iter->absolutePath().replace(":/", ":"));
        item->setIcon(0, provider.icon(QFileIconProvider::Folder));
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }

    model.reset(new QStandardItemModel(0, FW_COLUMNS_NUM));
    model->setHeaderData(FW_NAME, Qt::Horizontal,           tr("File Name"));
    model->setHeaderData(FW_SIZE, Qt::Horizontal,           tr("File Size"));
    tableFiles->setModel(model.data());
    tableFiles->setColumnWidth(0, 250);

    connect(treeFiles, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
    connect(treeFiles, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsed(QTreeWidgetItem*)));
    connect(treeFiles, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(itemClicked(QTreeWidgetItem*, int)));

    treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeFiles, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayTreeMenu(const QPoint&)));
}

files_widget::~files_widget()
{

}

void files_widget::itemExpanded(QTreeWidgetItem* item)
{
    if (item == allDirs || item == allFiles)
        return;

    QTreeWidgetItem* itemIter = item;
    while (itemIter->parent() != allDirs && itemIter->parent() != allFiles)
        itemIter = itemIter->parent();

    if (itemIter->parent() == allFiles)
        return;

    QString path = item->text(0) + "/";

    itemIter = item;
    while (itemIter->parent() != allDirs)
    {
        itemIter = itemIter->parent();
        path = itemIter->text(0) + "/" + path;
    }

    QDir dir(path);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);

    QFileInfoList::const_iterator iter;
    QIcon dirIcon(":/emule/common/FolderOpen.ico");
    for (iter = fileList.begin(); iter != fileList.end(); ++iter)
    {
        if (iter->isDir())
        {
            QTreeWidgetItem* newItem = new QTreeWidgetItem(item);
            newItem->setText(0, iter->fileName());
            newItem->setIcon(0, provider.icon(QFileIconProvider::Folder));

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
    if (item == allDirs || item == allFiles)
        return;

    QTreeWidgetItem* itemIter = item;
    while (itemIter->parent() != allDirs && itemIter->parent() != allFiles)
        itemIter = itemIter->parent();

    if (itemIter->parent() == allFiles)
        return;

    QString path = item->text(0) + "/";

    itemIter = item;
    while (itemIter->parent() != allDirs)
    {
        itemIter = itemIter->parent();
        path = itemIter->text(0) + "/" + path;
    }

    QDir dir(path);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files);

    for (int ii = model->rowCount()-1; ii >= 0; --ii)
        model->removeRow(ii);

    QFileInfoList::const_iterator iter;
    int row = 0;
    for (iter = fileList.begin(); iter != fileList.end(); ++iter)
    {
        model->insertRow(row);
        model->setData(model->index(row, FW_NAME), iter->fileName());
        model->setData(model->index(row, FW_SIZE), misc::friendlyUnit(iter->size()));
        QStandardItem* listItem = model->item(row, FW_NAME);
        listItem->setIcon(provider.icon(*iter));
        listItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        listItem->setCheckState(Qt::Unchecked);
    }
}

 void files_widget::displayTreeMenu(const QPoint&)
 {
 }
