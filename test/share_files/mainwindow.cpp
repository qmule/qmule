#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    //m_model = new TreeModel((DirNode*)m_sf.node("/home/apavlov"), TreeModel::All);
    m_model = new TreeModel((DirNode*)&m_sf.m_root, TreeModel::Dir);
    m_fileModel = new TreeModel((DirNode*)&m_sf.m_root, TreeModel::File);

    m_sf.share("/home/apavlov", false);
    ui->treeView->setModel(m_model);
    ui->tableView->setModel(m_fileModel);


    shareDir = new QAction(this);
    shareDir->setObjectName(QString::fromUtf8("shareDir"));
    shareDir->setText(tr("Share directory"));

    shareDirR = new QAction(this);
    shareDirR->setObjectName(QString::fromUtf8("shareDirR"));
    shareDirR->setText(tr("Share directory recursive"));

    unshareDir = new QAction(this);
    unshareDir->setObjectName(QString::fromUtf8("unshareDir"));
    unshareDir->setText(tr("Unshare directory"));

    unshareDirR = new QAction(this);
    unshareDirR->setObjectName(QString::fromUtf8("unshareDirR"));
    unshareDirR->setText(tr("Unshare directory recursive"));

    m_dir_menu = new QMenu(this);
    m_dir_menu->setObjectName(QString::fromUtf8("DirMenu"));
    m_dir_menu->setTitle(tr("Directory actions"));
    m_dir_menu->addAction(shareDir);
    m_dir_menu->addAction(shareDirR);
    m_dir_menu->addAction(unshareDir);
    m_dir_menu->addAction(unshareDirR);

    connect(shareDir,  SIGNAL(triggered()), this, SLOT(shareDirectory()));
    connect(shareDirR,  SIGNAL(triggered()), this, SLOT(shareDirectoryR()));
    connect(unshareDir,  SIGNAL(triggered()), this, SLOT(unshareDirectory()));
    connect(unshareDir,  SIGNAL(triggered()), this, SLOT(unshareDirectoryR()));

    connect(&m_sf, SIGNAL(changeNode(const FileNode*)), m_model, SLOT(changeNode(const FileNode*)));
    connect(&m_sf, SIGNAL(changeNode(const FileNode*)), m_fileModel, SLOT(changeNode(const FileNode*)));
    connect(&m_sf, SIGNAL(changeNode(const FileNode*)), m_fileModel, SLOT(changeNode(const FileNode*)));
    connect(&m_sf, SIGNAL(beginRemoveNode(const FileNode*)), m_fileModel, SLOT(beginRemoveNode(const FileNode*)));
    connect(&m_sf, SIGNAL(endRemoveNode()), m_fileModel, SLOT(endRemoveNode()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_model;
    delete m_fileModel;
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    m_fileModel->setRootNode(index);
}

void MainWindow::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex indx = ui->treeView->indexAt(pos);

    if (indx.isValid())
    {
        const FileNode* node = static_cast<const FileNode*>(indx.internalPointer());

        if (node->is_active())
        {
            shareDir->setEnabled(false);
            unshareDir->setEnabled(true);
        }
        else
        {
            shareDir->setEnabled(true);
            unshareDir->setEnabled(false);
        }

        m_dir_menu->exec(QCursor::pos());
    }
}

void MainWindow::shareDirectory()
{
   QModelIndex indx = ui->treeView->selectionModel()->currentIndex();

   if (indx.isValid())
   {
       qDebug() << "call shareDirectory";
       static_cast<FileNode*>(indx.internalPointer())->share(false);
   }
}

void MainWindow::shareDirectoryR()
{
    QModelIndex indx = ui->treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
        qDebug() << "call shareDirectoryR";
        static_cast<FileNode*>(indx.internalPointer())->share(true);
    }
}

void MainWindow::unshareDirectory()
{
    QModelIndex indx = ui->treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
        qDebug() << "call unshareDirectory";
        static_cast<FileNode*>(indx.internalPointer())->unshare(false);
    }
}

void MainWindow::unshareDirectoryR()
{
    QModelIndex indx = ui->treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
        qDebug() << "call unshareDirectoryR";
        static_cast<FileNode*>(indx.internalPointer())->unshare(true);
    }
}

void MainWindow::on_deleteButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Delete tranfer dialog"),
                                      tr("Delete file?"), QLineEdit::Normal,
                                      QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty())
    {
        qDebug() << "call delete " << text;
        FileNode* node = m_sf.node(text);

        if (node && (node != m_sf.root()))
        {
            QString hash = node->hash();
            qDebug() << "delete node on " << hash;
            m_sf.deleteTransfer(hash, false);
        }

    }
}

void MainWindow::on_addButton_clicked()
{
    qDebug() << "add clicked";
}
