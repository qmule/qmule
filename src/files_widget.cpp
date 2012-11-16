#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QClipboard>
#include "files_widget.h"
#include "session_fs_models/file_model.h"
#include "session_fs_models/dir_model.h"
#include "transport/session.h"

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

    tableView->horizontalHeader()->restoreState(pref.value("FilesWidget/FilesView").toByteArray());

    m_dir_model = new DirectoryModel(Session::instance()->root());
    m_file_model = new FilesModel(Session::instance()->root());

    treeView->setModel(m_dir_model);
    tableView->setModel(m_file_model);

    m_filesMenu = new QMenu(this);
    m_filesMenu->setObjectName(QString::fromUtf8("filesMenu"));
    m_filesMenu->setTitle(tr("Exchange files"));

    m_filesExchDir = new QAction(this);
    m_filesExchDir->setObjectName(QString::fromUtf8("filesExchDir"));
    m_filesExchDir->setText(tr("Exchange dir"));

    m_filesExchSubdir = new QAction(this);
    m_filesExchSubdir->setObjectName(QString::fromUtf8("filesExchSubdir"));
    m_filesExchSubdir->setText(tr("Exchange with subdirs"));

    m_filesUnexchDir = new QAction(this);
    m_filesUnexchDir->setObjectName(QString::fromUtf8("filesUnexchDir"));
    m_filesUnexchDir->setText(tr("Don't exchange dir"));

    m_filesUnexchSubdir = new QAction(this);
    m_filesUnexchSubdir->setObjectName(QString::fromUtf8("filesUnexchSubdir"));
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

    connect(m_filesExchDir,       SIGNAL(triggered()), this, SLOT(exchangeDir()));
    connect(m_filesExchSubdir,    SIGNAL(triggered()), this, SLOT(exchangeSubdir()));
    connect(m_filesUnexchDir,     SIGNAL(triggered()), this, SLOT(unexchangeDir()));
    connect(m_filesUnexchSubdir,  SIGNAL(triggered()), this, SLOT(unxchangeSubdir()));

    connect(Session::instance(), SIGNAL(changeNode(const FileNode*)), m_dir_model, SLOT(changeNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(changeNode(const FileNode*)), m_file_model, SLOT(changeNode(const FileNode*)));

    connect(Session::instance(), SIGNAL(beginRemoveNode(const FileNode*)), m_dir_model, SLOT(beginRemoveNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(endRemoveNode()), m_dir_model, SLOT(endRemoveNode()));
    connect(Session::instance(), SIGNAL(beginInsertNode(const FileNode*, int)), m_dir_model, SLOT(beginInsertNode(const FileNode*, int)));
    connect(Session::instance(), SIGNAL(endInsertNode()), m_dir_model, SLOT(endInsertNode()));

    connect(Session::instance(), SIGNAL(beginRemoveNode(const FileNode*)), m_file_model, SLOT(beginRemoveNode(const FileNode*)));
    connect(Session::instance(), SIGNAL(endRemoveNode()), m_file_model, SLOT(endRemoveNode()));
    connect(Session::instance(), SIGNAL(beginInsertNode(const FileNode*, int)), m_file_model, SLOT(beginInsertNode(const FileNode*, int)));
    connect(Session::instance(), SIGNAL(endInsertNode()), m_file_model, SLOT(endInsertNode()));


/*
    allFiles = new QTreeWidgetItem(treeFiles);
    allFiles->setText(0, tr("All exchange files"));
    allFiles->setIcon(0, QIcon(":/emule/files/all.ico"));
    allFiles->setExpanded(true);

    sharedDirs = new QTreeWidgetItem(allFiles);
    sharedDirs->setText(0, tr("Exchange folders"));
    sharedDirs->setIcon(0, provider.icon(QFileIconProvider::Folder));
    sharedDirs->setExpanded(true);
*/

/*
    connect(Session::instance()->get_ed2k_session(), SIGNAL(addedTransfer(Transfer)), this, SLOT(addedTransfer(Transfer)));
    connect(Session::instance()->get_ed2k_session(), SIGNAL(deletedTransfer(QString)), this, SLOT(deletedTransfer(QString)));

    connect(tableFiles->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(selectedFileChanged(const QItemSelection&, const QItemSelection&)));

    connect(checkForum, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));
    connect(checkSize, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));
    connect(btnCopy, SIGNAL(clicked()), this, SLOT(putToClipboard()));
    */
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

void files_widget::on_treeView_clicked(const QModelIndex &index)
{
    m_file_model->setRootNode(index);
}

void files_widget::exchangeDir()
{
    QModelIndex indx = treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
       qDebug() << "call shareDirectory";
       static_cast<FileNode*>(indx.internalPointer())->share(false);
    }
}

void files_widget::exchangeSubdir()
{
    QModelIndex indx = treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
       qDebug() << "call shareDirectoryR";
       static_cast<FileNode*>(indx.internalPointer())->share(true);
    }
}

void files_widget::unexchangeDir()
{
    QModelIndex indx = treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
       qDebug() << "call unshareDirectory";
       static_cast<FileNode*>(indx.internalPointer())->unshare(false);
    }
}

void files_widget::unxchangeSubdir()
{
    QModelIndex indx = treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
       qDebug() << "call unshareDirectoryR";
       static_cast<FileNode*>(indx.internalPointer())->unshare(true);
    }
}

void files_widget::closeEvent ( QCloseEvent * event )
{
}


void files_widget::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex indx = treeView->indexAt(pos);

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
    QModelIndex index = tableView->currentIndex();
    if (index.isValid())
    {
        if (m_file_model->active(index))
        {
            groupBox->setEnabled(true);
            editLink->setEnabled(true);
            btnCopy->setEnabled(true);
            checkForum->setEnabled(true);
            checkSize->setEnabled(true);
        }
        else
        {
            groupBox->setEnabled(false);
        }
    }
}
