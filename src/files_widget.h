#ifndef FILES_WIDGET_H
#define FILES_WIDGET_H

#include <QWidget>
#include <QFileIconProvider>
#include "ui_files_widget.h"
#include "misc.h"


class DirectoryModel;
class FilesModel;

class files_widget : public QWidget, public Ui::files_widget
{
    Q_OBJECT

public:
    files_widget(QWidget *parent = 0);
    ~files_widget();

private:
    QMenu*   filesMenu;
    QAction* filesExchDir;
    QAction* filesExchSubdir;
    QAction* filesUnexchDir;
    QAction* filesUnexchSubdir;

    DirectoryModel* m_dir_model;
    FilesModel*     m_file_model;
public slots:
    void putToClipboard();
private slots:
    void on_treeView_clicked(const QModelIndex &index);
};

#endif // FILES_WIDGET_H
