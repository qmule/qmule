#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../../src/silent_updater.h"
#include "../../src/taskbar_iface.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool winEvent(MSG * message, long * result);
private:
    Ui::MainWindow *ui;
    QScopedPointer<silent_updater> m_upd;
    QScopedPointer<taskbar_iface>  m_iface;
    unsigned int m_IDTaskbarButtonCreated;
private slots:
    void on_btn_clicked();

    void updateState(int state);
    void updateProgress(int value);
    void updateIcon(int index);
};

#endif // MAINWINDOW_H
