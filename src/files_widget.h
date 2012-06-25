#ifndef FILES_WIDGET_H
#define FILES_WIDGET_H

#include <QWidget>
#include <QFileIconProvider>
#include "ui_files_widget.h"

QT_BEGIN_NAMESPACE
class QStandardItemModel;
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
    QScopedPointer<QStandardItemModel> model;
    QFileIconProvider provider;

public:
    files_widget(QWidget *parent = 0);
    ~files_widget();

private slots:
    void itemExpanded(QTreeWidgetItem* item);
    void itemCollapsed(QTreeWidgetItem* item);
    void itemClicked(QTreeWidgetItem* item, int column);
    void displayTreeMenu(const QPoint&);
};

#endif // FILES_WIDGET_H
