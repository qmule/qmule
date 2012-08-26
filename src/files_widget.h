#ifndef FILES_WIDGET_H
#define FILES_WIDGET_H

#include <libed2k/file.hpp>
#include <transport/transfer.h>
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

    shared_map dirRules;
    QList<QString> fileRules;
    QMap<QString, QString> transferPath;

    QString saveDirPath;

    QMenu*   filesMenu;
    QAction* filesExchDir;
    QAction* filesExchSubdir;
    QAction* filesUnexchDir;
    QAction* filesUnexchSubdir;

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
    bool    isExchangeDir(QTreeWidgetItem* item);
    QString getDirPath(QTreeWidgetItem* item);
    void    generateSharedTree();
    void    exchangeSubdir(QFileInfoList& fileList);
    bool    partOfSharedPath(QString path);
    void    setExchangeStatus(QTreeWidgetItem* item, bool status);
    void    setChildExchangeStatus(QTreeWidgetItem* item, bool status);
    void    checkExchangeParentStatus(QTreeWidgetItem* curItem);
    void    shareDir(QString dirPath, bool bShare);
    void    checkBaseAdd(QString dirPath);
    void    checkBaseRemove(QString dirPath);
    void    removeLastSlash(QString& dirPath);
    void    addLastSlash(QString& dirPath);
    void    applyUnexchangeStatus(QString strPath, bool recursive);
    bool    findTreeItem(QTreeWidgetItem*& item, QString strPath);
    void    removeTransferPath(QString filePath);

public slots:
    void optionsChanged();
    void reshare();

private slots:
    void itemExpanded(QTreeWidgetItem* item);
    void itemCollapsed(QTreeWidgetItem* item);
    void currentItemChanged(QTreeWidgetItem* item, QTreeWidgetItem* olditem);
    void tableItemChanged(QStandardItem* item);
    void displayTreeMenu(const QPoint&);
    void exchangeDir();
    void exchangeSubdir();
    void unexchangeDir();
    void unxchangeSubdir();
    void applyChanges();
    void addedTransfer(Transfer transfer);
    void deletedTransfer(QString hash);
};

#endif // FILES_WIDGET_H
