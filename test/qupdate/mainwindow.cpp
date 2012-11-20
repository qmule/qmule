#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#ifdef Q_WS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_iface.reset(new taskbar_iface(this, 100));
    connect(ui->progressState, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState(int)));
    connect(ui->progressValue, SIGNAL(valueChanged(int)), this, SLOT(updateProgress(int)));
    connect(ui->iconOption, SIGNAL(currentIndexChanged(int)), this, SLOT(updateIcon(int)));
    m_IDTaskbarButtonCreated = RegisterWindowMessage(L"TaskbarButtonCreated");

    /*
    m_upd.reset(new silent_updater(0,0,0,0, this));
    connect(ui->pushButton, SIGNAL(clicked()), SLOT(on_btn_clicked()));
    QString old_path = QApplication::applicationFilePath() + QString(".old");

    if (QFile::exists(old_path))
    {
        QFile::remove(old_path);
    }
    */

}

#ifdef Q_WS_WIN
bool MainWindow::winEvent(MSG * message, long * result)
{
    if (message->message == m_IDTaskbarButtonCreated) {

        m_iface->initialize();
        m_iface->setState(winId(), taskbar_iface::S_NOPROGRESS);
    }

    return false;
}
#endif

void MainWindow::updateState(int state)
{
    switch(state)
    {
    case 1:
        m_iface->setState(winId(), taskbar_iface::S_INTERMEDIATE);
        break;
    case 2:
        m_iface->setState(winId(), taskbar_iface::S_NORM);
        ui->progressValue->setValue(1);
        ui->progressValue->setValue(0);
        break;
    case 3:
        m_iface->setState(winId(), taskbar_iface::S_ERROR);
        break;

    case 4:
        m_iface->setState(winId(), taskbar_iface::S_PAUSED);
        break;
    default:
        m_iface->setState(winId(), taskbar_iface::S_NOPROGRESS);
        ui->progressValue->setValue(1);
        ui->progressValue->setValue(0);
        break;
        }
}

void MainWindow::updateProgress(int value)
{
    m_iface->setProgress(winId(), value);
}

void MainWindow::updateIcon(int index)
{
    QString icon_res(":/icon_" + QVariant(index).toString());
    // a NULL icon will reset the current overlay icon
    m_iface->setIcon(winId(), index > 0 ? QIcon(icon_res) : QIcon(), "Description");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_clicked()
{

    //m_upd->start();
    return;

#ifdef Q_WS_WIN
    qDebug() << "btn clicked";
    qDebug() << "move" << QApplication::applicationFilePath();
    QString new_path = QApplication::applicationFilePath() + QString(".old");
    if (!MoveFileExA(QApplication::applicationFilePath().toLocal8Bit(), new_path.toLocal8Bit(), MOVEFILE_COPY_ALLOWED))
    {
        qDebug() << "error on move: " << GetLastError();
    }
    else
    {
        qDebug() << "move completed succefully";
        QFile nf("C:\\Windows\\notepad.exe");

        if (nf.copy(QApplication::applicationFilePath()))
        {
            qDebug() << "new program succesfully copied";
        }
    }
#endif
}
