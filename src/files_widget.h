#ifndef FILES_WIDGET_H
#define FILES_WIDGET_H

#include <libed2k/file.hpp>
#include <QWidget>
#include <QFileIconProvider>
#include "ui_files_widget.h"
#include "misc.h"

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QSortFilterProxyModel;
class QStandardItem;
QT_END_NAMESPACE


enum FW_Columns
{
    FW_NAME,
    FW_SIZE,
    FW_COLUMNS_NUM
};

class files_widget : public QWidget, public Ui::files_widget
{
    Q_OBJECT
    QTreeWidgetItem* allFiles;
    QTreeWidgetItem* allDirs;
    QTreeWidgetItem* sharedDirs;
    QScopedPointer<QStandardItemModel> model;
    QScopedPointer<QSortFilterProxyModel> filterModel;
    QFileIconProvider provider;

    shared_entry dirRules;
    QList<QString> fileRules;

    QMenu*   filesMenu;
    QAction* filesExchDir;
    QAction* filesExchSubdir;
    QAction* filesNotExchDir;
    QAction* filesNotExchSubdir;

    QFont usualFont;
    QFont boldFont;
    QIcon emuleFolder;

    bool bProcessFiles;

public:
    files_widget(QWidget *parent = 0);
    ~files_widget();

private:
    bool    isDirTreeItem(QTreeWidgetItem* item);
    bool    isSharedDirTreeItem(QTreeWidgetItem* item);
    QString getDirPath(QTreeWidgetItem* item);
    void    generateSharedTree();
    void    exchangeSubdir(QFileInfoList& fileList);
    bool    partOfSharedPath(QString path);
    void    setExchangeStatus(QTreeWidgetItem* item, bool status);
    void    setChildExchangeStatus(QTreeWidgetItem* item, bool status);
    void    checkExchangeParentStatus(QTreeWidgetItem* curItem);
    void    shareDir(QString dirPath, bool bShare);

private slots:
    void itemExpanded(QTreeWidgetItem* item);
    void itemCollapsed(QTreeWidgetItem* item);
    void itemClicked(QTreeWidgetItem* item, int column);
    void tableItemChanged(QStandardItem* item);
    void displayTreeMenu(const QPoint&);
    void exchangeDir();
    void exchangeSubdir();
    void notExchangeDir();
    void notExchangeSubdir();
    void applyChanges();
};

#endif // FILES_WIDGET_H
