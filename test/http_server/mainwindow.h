#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QTimer>
#include "httpserver.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void logStr(const QString& str);
    Ui::MainWindow *ui;
    QMenu*      m_mainMenu;
    QAction*    m_stopServer;
    QAction*    m_startServer;
    HttpServer* m_server;
private slots:
    void startServer();
    void stopServer();

    void newConnection();
};

#endif // MAINWINDOW_H
