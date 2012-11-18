#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QClipboard>

#include "files_widget.h"
#include "session_fs_models/file_model.h"
#include "session_fs_models/dir_model.h"
#include "session_fs_models/sort_model.h"
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

    if (!tableView->horizontalHeader()->restoreState(pref.value("FilesWidget/FilesView").toByteArray()))
    {
        tableView->horizontalHeader()->resizeSection(0, 20);
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

    m_filesMenu = new QMenu(this);
    m_filesMenu->setObjectName(QString::fromUtf8("filesMenu"));
    m_filesMenu->setTitle(tr("Exchange files"));

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

    m_filesMenu->addAction(m_filesExchDir);
    m_filesMenu->addAction(m_filesExchSubdir);
    m_filesMenu->addSeparator();
    m_filesMenu->addAction(m_filesUnexchDir);
    m_filesMenu->addAction(m_filesUnexchSubdir);

    connect(tableView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(on_tableViewSelChanged(const QItemSelection &, const QItemSelection &))
    );   

    connect(treeView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(on_treeViewSelChanged(const QItemSelection &, const QItemSelection &))
    );

    connect(m_filesExchDir,       SIGNAL(triggered()), this, SLOT(exchangeDir()));
    connect(m_filesExchSubdir,    SIGNAL(triggered()), this, SLOT(exchangeSubdir()));
    connect(m_filesUnexchDir,     SIGNAL(triggered()), this, SLOT(unexchangeDir()));
    connect(m_filesUnexchSubdir,  SIGNAL(triggered()), this, SLOT(unxchangeSubdir()));

    connect(Session::instance(), SIGNAL(changeNode(const FileNode*)), m_dir_model, SLOT(changeNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(changeNode(const FileNode*)), m_file_model, SLOT(changeNode(const FileNode*)));

    connect(m_file_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(on_changeRow(QModelIndex,QModelIndex)));

    connect(Session::instance(), SIGNAL(beginRemoveNode(const FileNode*)), m_dir_model, SLOT(beginRemoveNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(endRemoveNode()), m_dir_model, SLOT(endRemoveNode()));
    connect(Session::instance(), SIGNAL(beginInsertNode(const FileNode*)), m_dir_model, SLOT(beginInsertNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(endInsertNode()), m_dir_model, SLOT(endInsertNode()));

    connect(Session::instance(), SIGNAL(beginRemoveNode(const FileNode*)), m_file_model, SLOT(beginRemoveNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(endRemoveNode()), m_file_model, SLOT(endRemoveNode()));
    connect(Session::instance(), SIGNAL(beginInsertNode(const FileNode*)), m_file_model, SLOT(beginInsertNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(endInsertNode()), m_file_model, SLOT(endInsertNode()));

    connect(tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(sortChanged(int, Qt::SortOrder)));

    connect(treeView->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(sortChangedDirectory(int, Qt::SortOrder)));

    treeView->header()->setSortIndicator(BaseModel::DC_STATUS, Qt::AscendingOrder);
    tableView->horizontalHeader()->setSortIndicator(BaseModel::DC_NAME, Qt::AscendingOrder);
}

files_widget::~files_widget()
{    
    Preferences pref;
    pref.setValue("FilesWidget/Splitter", splitter_2->saveState());
    pref.setValue("FilesWidget/FilesView", tableView->horizontalHeader()->saveState());
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

void files_widget::exchangeDir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       qDebug() << "call shareDirectory";
       static_cast<FileNode*>(indx.internalPointer())->share(false);
    }
}

void files_widget::exchangeSubdir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       qDebug() << "call shareDirectoryR";
       static_cast<FileNode*>(indx.internalPointer())->share(true);
    }
}

void files_widget::unexchangeDir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       qDebug() << "call unshareDirectory";
       static_cast<FileNode*>(indx.internalPointer())->unshare(false);
    }
}

void files_widget::unxchangeSubdir()
{
    QModelIndex indx = sort2dir(treeView->selectionModel()->currentIndex());

    if (indx.isValid())
    {
       qDebug() << "call unshareDirectoryR";
       static_cast<FileNode*>(indx.internalPointer())->unshare(true);
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

QString files_widget::createLink(const QString& fileName, qint64 fileSize, const QString& fileHash, bool addForum, bool addSize)
{
    QString link = misc::toQStringU(libed2k::emule_collection::toLink(fileName.toUtf8().constData(), fileSize,
                                                         libed2k::md4_hash::fromString(fileHash.toStdString()), false));
    if (addForum)
    {
        link = "[u][b][url=" + link + "]" + fileName + "[/url][/b][/u]";
        if (addSize) link += " " + misc::friendlyUnit(fileSize);
    }

    return link;
}

void files_widget::switchLinkWidget(bool enable)
{
    if (enable)
    {
        groupBox->setEnabled(true);
        editLink->setEnabled(true);
        checkForum->setEnabled(true);
        checkSize->setEnabled(checkForum->isChecked());
        fillLinkWidget();
    }
    else
    {
        editLink->clear();
        groupBox->setEnabled(false);
    }
}

void files_widget::fillLinkWidget()
{
    QModelIndex index = sort2file(tableView->currentIndex());

    if (index.isValid())
    editLink->setPlainText(createLink(m_file_model->displayName(index),
                                      m_file_model->size(index),
                                      m_file_model->hash(index),
                                      checkForum->isChecked(),
                                      checkSize->isChecked()));
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

void files_widget::on_tableViewSelChanged(const QItemSelection &, const QItemSelection &)
{
    QModelIndex index = sort2file(tableView->currentIndex());

    if (index.isValid())
    {        
        switchLinkWidget(m_file_model->active(index));
    }    
}

void files_widget::on_treeViewSelChanged(const QItemSelection &, const QItemSelection &)
{
    QModelIndex index = sort2dir(treeView->currentIndex());

    if (index.isValid())
    {
        switchLinkWidget(false);
        m_file_model->setRootNode(index);
    }
}

void files_widget::sortChanged(int column, Qt::SortOrder order)
{
    m_sort_files_model->sort(column, order);
}

void files_widget::sortChangedDirectory(int column, Qt::SortOrder order)
{
    m_sort_dirs_model->sort(column, order);
}

void files_widget::on_editLink_textChanged()
{
    btnCopy->setDisabled(editLink->toPlainText().isEmpty());
}

void files_widget::on_checkForum_toggled(bool checked)
{
    checkSize->setEnabled(checked);
    fillLinkWidget();
}

void files_widget::on_checkSize_toggled(bool checked)
{
    Q_UNUSED(checked);
    fillLinkWidget();
}

void files_widget::on_btnCopy_clicked()
{
    putToClipboard();
}

void files_widget::on_changeRow(const QModelIndex& left, const QModelIndex& right)
{
    Q_UNUSED(right);
    // process current selection row changed state
    QModelIndex current = sort2file(tableView->currentIndex());

    if (left.isValid() && current.isValid() &&
        (left.column() == 0) &&         // process only first signal (FileModel emits second signal to refresh hash and errors)
        (left.row() == current.row()))  // check we stay on same row what was changed
    {
        switchLinkWidget(m_file_model->active(left) && !m_file_model->hash(left).isEmpty());
    }
}
