#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_server = new HttpServer(10, this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    // create actions
    m_startServer = new QAction(tr("&Start..."), this);
    //newAct->setShortcuts(QKeySequence::New);
    m_startServer->setStatusTip(tr("Start http server"));
    connect(m_startServer, SIGNAL(triggered()), this, SLOT(startServer()));

    m_stopServer = new QAction(tr("&Stop..."), this);
    //openAct->setShortcuts(QKeySequence::Open);
    m_stopServer->setStatusTip(tr("Stop http server"));
    m_stopServer->setEnabled(false);
    connect(m_stopServer, SIGNAL(triggered()), this, SLOT(stopServer()));
    m_mainMenu = menuBar()->addMenu(tr("&Server control"));
    m_mainMenu->addAction(m_startServer);
    m_mainMenu->addAction(m_stopServer);
}

MainWindow::~MainWindow()
{
}

void MainWindow::logStr(const QString& str)
{
    ui->text->appendPlainText(str);
}

void MainWindow::startServer()
{
    if (!m_server->listen(QHostAddress::Any, 8080))
    {
        logStr("Unable to start server:");
        logStr(m_server->errorString());
    }
    else
    {
        ui->text->appendPlainText("Server started succefully");
        m_startServer->setEnabled(false);
        m_stopServer->setEnabled(true);
    }
}

void MainWindow::stopServer()
{
    m_server->close();
    m_startServer->setEnabled(true);
    m_stopServer->setEnabled(false);
    logStr("Server stopped");
}

void MainWindow::newConnection()
{
    logStr("New connection!");
}
