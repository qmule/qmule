#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <windows.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_upd.reset(new silent_updater(0,0,0,0, this));
    connect(ui->pushButton, SIGNAL(clicked()), SLOT(on_btn_clicked()));
    QString old_path = QApplication::applicationFilePath() + QString(".old");

    if (QFile::exists(old_path))
    {
        QFile::remove(old_path);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_clicked()
{
    m_upd->start();
    return;
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
}
