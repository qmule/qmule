#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include "share_files.h"
#include "tree.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_treeView_clicked(const QModelIndex &index);

    void on_treeView_customContextMenuRequested(const QPoint &pos);

private:
    Ui::MainWindow *ui;
    Session m_sf;
    TreeModel* m_model;
    TreeModel* m_fileModel;
    QMenu*      m_dir_menu;
    QAction*    shareDir;
    QAction*    shareDirR;
    QAction*    unshareDir;
    QAction*    unshareDirR;
private slots:
    void shareDirectory();
    void shareDirectoryR();
    void unshareDirectory();
    void unshareDirectoryR();
    void on_deleteButton_clicked();
    void on_addButton_clicked();
};

#endif // MAINWINDOW_H
