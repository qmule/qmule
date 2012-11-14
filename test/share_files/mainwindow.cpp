#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    m_model = new DirectoryModel((DirNode*)&m_sf.m_root);
    m_fileModel = new FileModel((DirNode*)&m_sf.m_root);

    m_sf.share("/home/apavlov", false);
    ui->treeView->setModel(m_model);
    ui->tableView->setModel(m_fileModel);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
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

    removeD = new QAction(this);
    removeD->setObjectName(QString::fromUtf8("remove D"));
    removeD->setText(tr("Remove directory"));

    m_dir_menu = new QMenu(this);
    m_dir_menu->setObjectName(QString::fromUtf8("DirMenu"));
    m_dir_menu->setTitle(tr("Directory actions"));
    m_dir_menu->addAction(shareDir);
    m_dir_menu->addAction(shareDirR);
    m_dir_menu->addAction(unshareDir);
    m_dir_menu->addAction(unshareDirR);
    m_dir_menu->addAction(removeD);


    removeF = new QAction(this);
    removeF->setObjectName(QString::fromUtf8("removeFile"));
    removeF->setText(tr("Remove file"));

    m_file_menu = new QMenu(this);
    m_file_menu->setObjectName(QString::fromUtf8("FileMenu"));
    m_file_menu->setTitle(tr("File actions"));

    m_file_menu->addAction(removeF);


    connect(shareDir,  SIGNAL(triggered()), this, SLOT(shareDirectory()));
    connect(shareDirR,  SIGNAL(triggered()), this, SLOT(shareDirectoryR()));
    connect(unshareDir,  SIGNAL(triggered()), this, SLOT(unshareDirectory()));
    connect(unshareDirR,  SIGNAL(triggered()), this, SLOT(unshareDirectoryR()));
    connect(removeF,  SIGNAL(triggered()), this, SLOT(removeFile()));
    connect(removeD,  SIGNAL(triggered()), this, SLOT(removeDir()));

    connect(&m_sf, SIGNAL(changeNode(const FileNode*)), m_model, SLOT(changeNode(const FileNode*)));
    connect(&m_sf, SIGNAL(changeNode(const FileNode*)), m_fileModel, SLOT(changeNode(const FileNode*)));


    connect(&m_sf, SIGNAL(beginRemoveNode(const FileNode*)), m_model, SLOT(beginRemoveNode(const FileNode*)));
    connect(&m_sf, SIGNAL(endRemoveNode()), m_model, SLOT(endRemoveNode()));
    connect(&m_sf, SIGNAL(beginInsertNode(const FileNode*, int)), m_model, SLOT(beginInsertNode(const FileNode*, int)));
    connect(&m_sf, SIGNAL(endInsertNode()), m_model, SLOT(endInsertNode()));

    connect(&m_sf, SIGNAL(beginRemoveNode(const FileNode*)), m_fileModel, SLOT(beginRemoveNode(const FileNode*)));
    connect(&m_sf, SIGNAL(endRemoveNode()), m_fileModel, SLOT(endRemoveNode()));
    connect(&m_sf, SIGNAL(beginInsertNode(const FileNode*, int)), m_fileModel, SLOT(beginInsertNode(const FileNode*, int)));
    connect(&m_sf, SIGNAL(endInsertNode()), m_fileModel, SLOT(endInsertNode()));    

    connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(testOnDeleteSlot(QModelIndex,int,int)));
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
        const DirNode* node = static_cast<const DirNode*>(indx.internalPointer());

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

void MainWindow::removeFile()
{
    QModelIndex indx = ui->tableView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
        qDebug() << "call removeFile";
        FileNode* node = static_cast<FileNode*>(indx.internalPointer());
        DirNode* parent = node->m_parent;

        if (parent)
        {
            qDebug() << "delete node call";
            parent->delete_node(node);
        }
    }
}

void MainWindow::removeDir()
{
    QModelIndex indx = ui->treeView->selectionModel()->currentIndex();

    if (indx.isValid())
    {
        qDebug() << "call removeFile";
        FileNode* node = static_cast<FileNode*>(indx.internalPointer());
        DirNode* parent = node->m_parent;

        if (parent)
        {
            qDebug() << "delete directory node call";
            parent->delete_node(node);
        }
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
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add file dialog"),
                                      tr("Generate test file?"), QLineEdit::Normal,
                                      QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty())
    {
        QFile data(text);

        if (data.open(QFile::WriteOnly | QFile::Truncate))
        {
             QTextStream out(&data);
             out << "Result: " << qSetFieldWidth(10) << left << 3.14 << 2.7;
        }

        FileNode* node = m_sf.node(text);

        if (node)
        {
            qDebug() << "add node " << node->filepath();
        }

    }
}

void MainWindow::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    qDebug() << "table view menu";
    QModelIndex indx = ui->tableView->indexAt(pos);

    if (indx.isValid())
    {
        m_file_menu->exec(QCursor::pos());
    }
}

void MainWindow::on_pushButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add directory dialog"),
                                      tr("Generate directory "), QLineEdit::Normal,
                                      QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty())
    {
        QDir d("/home/apavlov/work/");

        if (d.mkdir(text))
        {
            for (size_t n = 0; n < 5; ++n)
            {
                QFile data(QString("/home/apavlov/work") + QDir::separator() + text + QDir::separator() +
                           QString("file") + QString::number(n));

                if (data.open(QFile::WriteOnly | QFile::Truncate))
                {
                     QTextStream out(&data);
                     out << "Result: " << qSetFieldWidth(10) << left << 3.14 << 2.7;
                }
            }

            FileNode* node = m_sf.node(QString("/home/apavlov/work") + QDir::separator() + text);

            if (node)
            {
                qDebug() << "add dir node " << node->filepath();
            }
        }

    }
}

void MainWindow::testOnDeleteSlot(const QModelIndex& indx, int start, int end)
{
    if (indx.isValid())
    {
        m_fileModel->setRootNode(indx);
    }
}
