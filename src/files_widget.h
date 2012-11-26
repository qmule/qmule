#ifndef FILES_WIDGET_H
#define FILES_WIDGET_H

#include <QWidget>
#include <QFileIconProvider>
#include <QSplitter>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include "ui_files_widget.h"
#include "misc.h"


class DirectoryModel;
class FilesModel;
class SessionFilesSort;
class SessionDirectoriesSort;

class files_widget : public QWidget, public Ui::files_widget
{
    Q_OBJECT

public:
    files_widget(QWidget *parent = 0);
    ~files_widget();

private:    
    QMenu*   m_filesMenu;
    QAction* m_openFolder;
    QAction* m_filesExchDir;
    QAction* m_filesExchSubdir;
    QAction* m_filesUnexchDir;
    QAction* m_filesUnexchSubdir;

    DirectoryModel* m_dir_model;
    FilesModel*     m_file_model;
    SessionFilesSort* m_sort_files_model;
    SessionDirectoriesSort* m_sort_dirs_model;
    QModelIndex sort2dir(const QModelIndex& index) const;
    QModelIndex sort2file(const QModelIndex& index) const;
    QString createLink(const QString& fileName, qint64 fileSize, const QString& fileHash, bool addForum, bool addSize);
    void switchLinkWidget(const QStringList&);
    void fillLinkWidget(const QStringList&);
    QStringList generateLinks();
public slots:
    void putToClipboard();
private slots:
    void openFolder();
    void exchangeDir();
    void exchangeSubdir();
    void unexchangeDir();
    void unxchangeSubdir();
    void on_treeView_customContextMenuRequested(const QPoint &pos);
    void on_tableViewSelChanged(const QItemSelection &, const QItemSelection &);
    void on_treeViewSelChanged(const QItemSelection &, const QItemSelection &);
    void sortChanged(int, Qt::SortOrder);
    void sortChangedDirectory(int, Qt::SortOrder);
    void on_editLink_textChanged();
    void on_checkForum_toggled(bool checked);
    void on_checkSize_toggled(bool checked);
    void on_btnCopy_clicked();
    void on_changeRow(const QModelIndex& left, const QModelIndex& right);
    void displayHSMenu(const QPoint&);
};

#endif // FILES_WIDGET_H
