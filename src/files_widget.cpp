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

    m_dir_model = new DirectoryModel(Session::instance()->root());
    m_file_model = new FilesModel(Session::instance()->root());

    treeView->setModel(m_dir_model);
    tableView->setModel(m_file_model);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);


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
    */
}

files_widget::~files_widget()
{
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
