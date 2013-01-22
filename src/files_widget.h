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
class PathModel;
class SFModel;
class SessionFilesSort;
class SessionDirectoriesSort;
class PathsSort;

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
    QAction* m_reloadDirectory;
    QAction* m_openFile;
    QAction* m_openSumFile;

    DirectoryModel* m_dir_model;
    FilesModel*     m_file_model;
    PathModel*      m_path_model;
    PathsSort*      m_path_sort;
    SFModel*        m_sum_file_model;
    SessionFilesSort* m_sum_sort_files_model;

    SessionFilesSort* m_sort_files_model;
    SessionDirectoriesSort* m_sort_dirs_model;
    QModelIndex sort2dir(const QModelIndex& index) const;
    QModelIndex sort2file(const QModelIndex& index) const;
    QModelIndex sort2dir_sum(const QModelIndex& index) const;
    QModelIndex sort2file_sum(const QModelIndex& index) const;
    QString createLink(const QString& fileName, qint64 fileSize, const QString& fileHash, bool addForum, bool addSize);
    void switchLinkWidget(const QStringList&);
    void fillLinkWidget(const QStringList&);
    QStringList generateLinks();        // files browser
    QStringList generateLinksSum();     // summary browser
    QStringList generateLinksByTab();   // for 0 files, for 1 summary
public slots:
    void putToClipboard();
private slots:
    void openFolder();
    void exchangeDir();
    void exchangeSubdir();
    void unexchangeDir();
    void unxchangeSubdir();
    void reloadDir();
    void on_treeView_customContextMenuRequested(const QPoint &pos);
    void on_tableViewSelChanged(const QItemSelection &, const QItemSelection &);
    void on_treeViewSelChanged(const QItemSelection &, const QItemSelection &);
    void on_tableViewPathsSumSelChanged(const QItemSelection&, const QItemSelection&);
    void on_tableViewFilesSumSelChanged(const QItemSelection&, const QItemSelection&);
    void sortChanged(int, Qt::SortOrder);
    void sortChangedDirectory(int, Qt::SortOrder);
    void paths_sortChanged(int, Qt::SortOrder);
    void files_sortChanged(int, Qt::SortOrder);
    void on_editLink_textChanged();
    void on_checkForum_toggled(bool checked);
    void on_checkSize_toggled(bool checked);
    void on_btnCopy_clicked();
    void on_changeRow(const QModelIndex& left, const QModelIndex& right);
    void displayHSMenu(const QPoint&);
    void displayHSMenuSummary(const QPoint&);
    void on_tabWidget_currentChanged(int index);
    void openFile(const QModelIndex& index);
    void openSelectedFile();
    void openSumFile(const QModelIndex& index);
    void openSelectedSumFile();
};

#endif // FILES_WIDGET_H
