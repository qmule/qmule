#include "mainwindow.h"
#include "ui_mainwindow.h"

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
