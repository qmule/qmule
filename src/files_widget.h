#ifndef FILES_WIDGET_H
#define FILES_WIDGET_H

#include <QWidget>
#include <QFileIconProvider>
#include <QSplitter>
#include <QItemSelection>
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
    QMenu*   m_filesMenu;
    QAction* m_filesExchDir;
    QAction* m_filesExchSubdir;
    QAction* m_filesUnexchDir;
    QAction* m_filesUnexchSubdir;

    DirectoryModel* m_dir_model;
    FilesModel*     m_file_model;
protected:
    virtual void closeEvent ( QCloseEvent * event);
public slots:
    void putToClipboard();
private slots:
    void on_treeView_clicked(const QModelIndex &index);
    void exchangeDir();
    void exchangeSubdir();
    void unexchangeDir();
    void unxchangeSubdir();
    void on_treeView_customContextMenuRequested(const QPoint &pos);
    void on_tableViewSelChanged(const QItemSelection &, const QItemSelection &);
};

#endif // FILES_WIDGET_H
