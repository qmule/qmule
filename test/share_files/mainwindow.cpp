#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    //m_model = new TreeModel((DirNode*)m_sf.node("/home/apavlov"), TreeModel::All);
    m_model = new TreeModel((DirNode*)&m_sf.m_root, TreeModel::All);
    ui->treeView->setModel(m_model);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_model;
}
