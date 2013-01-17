#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QClipboard>
#include <QDesktopServices>

#include "files_widget.h"
#include "session_fs_models/file_model.h"
#include "session_fs_models/dir_model.h"
#include "session_fs_models/sort_model.h"
#include "session_fs_models/path_model.h"
#include "session_fs_models/shared_files_model.h"
#include "transport/session.h"

#include <libed2k/file.hpp>

files_widget::files_widget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    Preferences pref;

    if (!splitter_2->restoreState(pref.value("FilesWidget/Splitter").toByteArray()))
    {
        QList<int> sz;
        sz << 100 << 500;
        splitter_2->setSizes(sz);
    }

    m_dir_model = new DirectoryModel(Session::instance()->root());
    m_file_model = new FilesModel(Session::instance()->root());


    m_sort_files_model = new SessionFilesSort(this);
    m_sort_files_model->setSourceModel(m_file_model);
    m_sort_files_model->setDynamicSortFilter(true);

    m_sort_dirs_model = new SessionDirectoriesSort(this);
    m_sort_dirs_model->setSourceModel(m_dir_model);

    treeView->setModel(m_sort_dirs_model);
    tableView->setModel(m_sort_files_model);    

    if (!tableView->horizontalHeader()->restoreState(pref.value("FilesWidget/FilesView").toByteArray()))
    {
        tableView->setColumnWidth(0, 20);
        tableView->setColumnWidth(1, 400);
    }

    treeView->header()->restoreState(pref.value("FilesWidget/DirectoriesView").toByteArray());

    m_filesMenu = new QMenu(this);
    m_filesMenu->setObjectName(QString::fromUtf8("filesMenu"));
    m_filesMenu->setTitle(tr("Exchange files"));

    m_openFolder = new QAction(this);
    m_openFolder->setObjectName(QString::fromUtf8("openFolder"));
    m_openFolder->setIcon(QIcon(":/emule/common/folder_open.ico"));
    m_openFolder->setText(tr("Open folder"));

    m_filesExchDir = new QAction(this);
    m_filesExchDir->setObjectName(QString::fromUtf8("filesExchDir"));
    m_filesExchDir->setIcon(QIcon(":/emule/common/folder_share.ico"));
    m_filesExchDir->setText(tr("Exchange dir"));

    m_filesExchSubdir = new QAction(this);
    m_filesExchSubdir->setObjectName(QString::fromUtf8("filesExchSubdir"));
    m_filesExchSubdir->setIcon(QIcon(":/emule/common/folder_share.ico"));
    m_filesExchSubdir->setText(tr("Exchange with subdirs"));

    m_filesUnexchDir = new QAction(this);
    m_filesUnexchDir->setObjectName(QString::fromUtf8("filesUnexchDir"));
    m_filesUnexchDir->setIcon(QIcon(":/emule/common/folder_unshare.ico"));
    m_filesUnexchDir->setText(tr("Don't exchange dir"));

    m_filesUnexchSubdir = new QAction(this);
    m_filesUnexchSubdir->setObjectName(QString::fromUtf8("filesUnexchSubdir"));
    m_filesUnexchSubdir->setIcon(QIcon(":/emule/common/folder_unshare.ico"));
    m_filesUnexchSubdir->setText(tr("Don't exchange with subdirs"));

    m_reloadDirectory = new QAction(this);
    m_reloadDirectory->setObjectName(QString::fromUtf8("reloadDir"));
    m_reloadDirectory->setIcon(QIcon(":/emule/common/folder_reload.ico"));
    m_reloadDirectory->setText(tr("Reload directory"));

    m_filesMenu->addAction(m_openFolder);
    m_filesMenu->addSeparator();
    m_filesMenu->addAction(m_filesExchDir);
    m_filesMenu->addAction(m_filesExchSubdir);
    m_filesMenu->addSeparator();
    m_filesMenu->addAction(m_filesUnexchDir);
    m_filesMenu->addAction(m_filesUnexchSubdir);
    m_filesMenu->addSeparator();
    m_filesMenu->addAction(m_reloadDirectory);

    connect(tableView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(on_tableViewSelChanged(const QItemSelection &, const QItemSelection &))
    );   

    connect(treeView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(on_treeViewSelChanged(const QItemSelection &, const QItemSelection &))
    );

    connect(m_openFolder,         SIGNAL(triggered()), this, SLOT(openFolder()));
    connect(m_filesExchDir,       SIGNAL(triggered()), this, SLOT(exchangeDir()));
    connect(m_filesExchSubdir,    SIGNAL(triggered()), this, SLOT(exchangeSubdir()));
    connect(m_filesUnexchDir,     SIGNAL(triggered()), this, SLOT(unexchangeDir()));
    connect(m_filesUnexchSubdir,  SIGNAL(triggered()), this, SLOT(unxchangeSubdir()));
    connect(m_reloadDirectory,    SIGNAL(triggered()), this, SLOT(reloadDir()));

    connect(m_file_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(on_changeRow(QModelIndex,QModelIndex)));

    connect(tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(sortChanged(int, Qt::SortOrder)));

    connect(treeView->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(sortChangedDirectory(int, Qt::SortOrder)));

    tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(displayHSMenu(const QPoint&)));

    treeView->header()->setSortIndicator(BaseModel::DC_STATUS, Qt::AscendingOrder);
    tableView->horizontalHeader()->setSortIndicator(BaseModel::DC_NAME, Qt::AscendingOrder);        

    // ======== summary page ==============

    if (!splitter_3->restoreState(pref.value("FilesWidget/SplitterSummary").toByteArray()))
    {
        QList<int> sz;
        sz << 100 << 500;
        splitter_3->setSizes(sz);
    }

    if (!tableView_files->horizontalHeader()->restoreState(pref.value("FilesWidget/FilesViewSummary").toByteArray()))
    {
        tableView_files->setColumnWidth(0, 20);
        tableView_files->setColumnWidth(1, 400);
    }

    tableView_paths->horizontalHeader()->restoreState(pref.value("FilesWidget/DirectoriesViewSummary").toByteArray());

    m_path_model = new PathModel(this);
    m_path_sort = new PathsSort(this);
    m_path_sort->setSourceModel(m_path_model);
    m_path_sort->setDynamicSortFilter(false);

    tableView_paths->setModel(m_path_sort);    

    m_sum_file_model = new SFModel(this);
    m_sum_sort_files_model = new SessionFilesSort(this);
    m_sum_sort_files_model->setSourceModel(m_sum_file_model);
    m_sum_sort_files_model->setDynamicSortFilter(true); // for performance dynamic sort off

    tableView_files->setModel(m_sum_sort_files_model);

    connect(tableView_paths->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(on_tableViewPathsSumSelChanged(QItemSelection,QItemSelection)));

    connect(tableView_files->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(on_tableViewFilesSumSelChanged(const QItemSelection &, const QItemSelection &)));

    connect(tableView_paths->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(paths_sortChanged(int, Qt::SortOrder)));

    connect(tableView_files->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(files_sortChanged(int, Qt::SortOrder)));

    tableView_files->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView_files->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(displayHSMenuSummary(const QPoint&)));

    // pre-sort summary models
    tableView_paths->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
    tableView_files->horizontalHeader()->setSortIndicator(BaseModel::DC_NAME, Qt::AscendingOrder);
}

files_widget::~files_widget()
{    
    Preferences pref;
    pref.setValue("FilesWidget/Splitter", splitter_2->saveState());
    pref.setValue("FilesWidget/SplitterSummary", splitter_3->saveState());
    pref.setValue("FilesWidget/FilesView", tableView->horizontalHeader()->saveState());
    pref.setValue("FilesWidget/FilesViewSummary", tableView_files->horizontalHeader()->saveState());
    pref.setValue("FilesWidget/DirectoriesView", treeView->header()->saveState());
    pref.setValue("FilesWidget/DirectoriesViewSummary", tableView_paths->horizontalHeader()->saveState());
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

void files_widget::openFolder()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
        QDesktopServices::openUrl(static_cast<FileNode*>(indx.internalPointer())->filepath());
    }
}

void files_widget::exchangeDir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       QApplication::setOverrideCursor(Qt::WaitCursor);
       qDebug() << "call shareDirectory";
       static_cast<FileNode*>(indx.internalPointer())->share(false);
       QApplication::restoreOverrideCursor();
    }
}

void files_widget::exchangeSubdir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       QApplication::setOverrideCursor(Qt::WaitCursor);
       qDebug() << "call shareDirectoryR";
       static_cast<FileNode*>(indx.internalPointer())->share(true);
       QApplication::restoreOverrideCursor();
    }
}

void files_widget::unexchangeDir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       QApplication::setOverrideCursor(Qt::WaitCursor);
       qDebug() << "call unshareDirectory";
       static_cast<FileNode*>(indx.internalPointer())->unshare(false);
       QApplication::restoreOverrideCursor();
    }
}

void files_widget::unxchangeSubdir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       QApplication::setOverrideCursor(Qt::WaitCursor);
       qDebug() << "call unshareDirectoryR";
       static_cast<FileNode*>(indx.internalPointer())->unshare(true);
       QApplication::restoreOverrideCursor();
    }
}

void files_widget::reloadDir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       QApplication::setOverrideCursor(Qt::WaitCursor);
       qDebug() << "call reload dir";
       static_cast<DirNode*>(indx.internalPointer())->populate(true);
       //static_cast<DirNode*>(indx.internalPointer())->deleteTransfer();
       QApplication::restoreOverrideCursor();
    }
}

QModelIndex files_widget::sort2dir(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == m_sort_dirs_model);
        return m_sort_dirs_model->mapToSource(index);
    }

    return QModelIndex();
}

QModelIndex files_widget::sort2file(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == m_sort_files_model);
        return m_sort_files_model->mapToSource(index);
    }

    return QModelIndex();
}

QModelIndex files_widget::sort2dir_sum(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == m_path_sort);
        return m_path_sort->mapToSource(index);
    }

    return QModelIndex();
}

QModelIndex files_widget::sort2file_sum(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == m_sum_sort_files_model);
        return m_sum_sort_files_model->mapToSource(index);
    }

    return QModelIndex();
}

QString files_widget::createLink(const QString& fileName, qint64 fileSize, const QString& fileHash, bool addForum, bool addSize)
{
    QString link;

    if (!fileHash.isEmpty())
    {
        link = misc::toQStringU(libed2k::emule_collection::toLink(fileName.toUtf8().constData(), fileSize,
                                                             libed2k::md4_hash::fromString(fileHash.toStdString()), true));
        if (addForum)
        {
            link = "[u][b][url=" + link + "]" + fileName + "[/url][/b][/u]";
            if (addSize) link += " " + misc::friendlyUnit(fileSize);
        }
    }

    return link;
}

void files_widget::switchLinkWidget(const QStringList& links)
{
    if (!links.isEmpty())
    {
        groupBox->setEnabled(true);
        editLink->setEnabled(true);
        checkForum->setEnabled(true);
        checkSize->setEnabled(checkForum->isChecked());
        fillLinkWidget(links);
    }
    else
    {
        editLink->clear();
        groupBox->setEnabled(false);
    }
}

void files_widget::fillLinkWidget(const QStringList& links)
{       
    editLink->clear();

    foreach(const QString& s, links)
    {
        editLink->appendPlainText(s);
    }
}

QStringList files_widget::generateLinks()
{
    QStringList list;
    foreach(const QModelIndex& i, tableView->selectionModel()->selectedRows())
    {
        QModelIndex index = sort2file(i);

        if (index.isValid() && m_file_model->active(index) && !m_file_model->hash(index).isEmpty())
        {
            list << createLink(m_file_model->displayName(index),
                                          m_file_model->size(index),
                                          m_file_model->hash(index),
                                          checkForum->isChecked(),
                                          checkSize->isChecked());
        }
    }

    return list;
}

QStringList files_widget::generateLinksSum()
{
    QStringList list;
    foreach(const QModelIndex& i, tableView_files->selectionModel()->selectedRows())
    {
        QModelIndex index = sort2file_sum(i);

        if (index.isValid() && m_sum_file_model->active(index) && !m_sum_file_model->hash(index).isEmpty())
        {
            list << createLink(m_sum_file_model->displayName(index),
                                          m_sum_file_model->size(index),
                                          m_sum_file_model->hash(index),
                                          checkForum->isChecked(),
                                          checkSize->isChecked());
        }
    }

    return list;
}

QStringList files_widget::generateLinksByTab()
{
    QStringList res;

    switch(tabWidget->currentIndex())
    {
    case 0:
        res = generateLinks();
        break;
    case 1:
        res = generateLinksSum();
        break;
    default:
        Q_ASSERT(false);
    }

    return res;
}

void files_widget::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex indx = sort2dir(treeView->indexAt(pos));

    if (indx.isValid())
    {
        const DirNode* node = static_cast<const DirNode*>(indx.internalPointer());

        if (node->is_active())
        {
            m_filesExchDir->setEnabled(false);
            m_filesUnexchDir->setEnabled(true);
        }
        else
        {
            m_filesExchDir->setEnabled(true);
            m_filesUnexchDir->setEnabled(false);
        }

        m_filesMenu->exec(QCursor::pos());
    }
}

void files_widget::on_tableViewSelChanged(const QItemSelection& sel, const QItemSelection& dsel)
{    
    switchLinkWidget(generateLinks());
}

void files_widget::on_treeViewSelChanged(const QItemSelection&, const QItemSelection&)
{   
    QModelIndex index = sort2dir(treeView->currentIndex());

    if (index.isValid())
    {
        switchLinkWidget(QStringList());
        m_file_model->setRootNode(index);
    }
}

void files_widget::on_tableViewPathsSumSelChanged(const QItemSelection&, const QItemSelection&)
{
    QModelIndex index = sort2dir_sum(tableView_paths->currentIndex());

    if (index.isValid())
    {
        switchLinkWidget(QStringList());
        m_sum_file_model->setFilter(m_path_model->filepath(index));
    }
}

void files_widget::on_tableViewFilesSumSelChanged(const QItemSelection&, const QItemSelection&)
{
    switchLinkWidget(generateLinksSum());
}

void files_widget::sortChanged(int column, Qt::SortOrder order)
{
    m_sort_files_model->sort(column, order);
}

void files_widget::sortChangedDirectory(int column, Qt::SortOrder order)
{
    m_sort_dirs_model->sort(column, order);
}

void files_widget::paths_sortChanged(int column, Qt::SortOrder order)
{
    m_path_sort->sort(column, order);
}

void files_widget::files_sortChanged(int column, Qt::SortOrder order)
{
    m_sum_sort_files_model->sort(column, order);
}

void files_widget::on_editLink_textChanged()
{
    btnCopy->setDisabled(editLink->toPlainText().isEmpty());
}

void files_widget::on_checkForum_toggled(bool checked)
{
    checkSize->setEnabled(checked);
    fillLinkWidget(generateLinksByTab());
}

void files_widget::on_checkSize_toggled(bool checked)
{
    Q_UNUSED(checked);
    fillLinkWidget(generateLinksByTab());
}

void files_widget::on_btnCopy_clicked()
{
    putToClipboard();
}

void files_widget::on_changeRow(const QModelIndex& left, const QModelIndex& right)
{
    Q_UNUSED(right);    

    // process only first signal (FileModel emits second signal to refresh hash and errors)
    if (left.isValid() && (left.column() == 0))
    {
        switchLinkWidget(generateLinks());
    }
}

void files_widget::displayHSMenu(const QPoint&)
{
    QMenu hideshowColumn(this);
    hideshowColumn.setTitle(tr("Column visibility"));
    QList<QAction*> actions;

    for (int i=0; i < m_file_model->columnCount(); ++i)
    {
        QAction *myAct = hideshowColumn.addAction(
            m_file_model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        myAct->setCheckable(true);
        myAct->setChecked(!tableView->isColumnHidden(i));
        actions.append(myAct);
    }

    // Call menu
    QAction *act = hideshowColumn.exec(QCursor::pos());

    if (act)
    {
        int col = actions.indexOf(act);
        Q_ASSERT(col >= 0);
        tableView->setColumnHidden(col, !tableView->isColumnHidden(col));
        if (!tableView->isColumnHidden(col) && tableView->columnWidth(col) <= 5)
            tableView->setColumnWidth(col, 100);
    }
}

void files_widget::displayHSMenuSummary(const QPoint&)
{
    QMenu hideshowColumn(this);
    hideshowColumn.setTitle(tr("Column visibility"));
    QList<QAction*> actions;

    for (int i=0; i < m_sum_file_model->columnCount(); ++i)
    {
        QAction *myAct = hideshowColumn.addAction(
            m_sum_file_model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        myAct->setCheckable(true);
        myAct->setChecked(!tableView_files->isColumnHidden(i));
        actions.append(myAct);
    }

    // Call menu
    QAction *act = hideshowColumn.exec(QCursor::pos());

    if (act)
    {
        int col = actions.indexOf(act);
        Q_ASSERT(col >= 0);
        tableView_files->setColumnHidden(col, !tableView_files->isColumnHidden(col));
        if (!tableView_files->isColumnHidden(col) && tableView_files->columnWidth(col) <= 5)
            tableView_files->setColumnWidth(col, 100);
    }
}


void files_widget::on_tabWidget_currentChanged(int index)
{
    tableView->clearSelection();
    tableView_files->clearSelection();
}
