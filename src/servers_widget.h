#ifndef SERVERS_WIDGET_H
#define SERVERS_WIDGET_H

#include <QWidget>
#include "ui_servers_widget.h"

class servers_table_model;
class QSortFilterProxyModel;
class QMenu;
class QAction;

class servers_widget : public QWidget, private Ui::servers_widget
{
    Q_OBJECT
private:
    QSortFilterProxyModel*  m_sort_model;
    servers_table_model*    m_smodel;
    QMenu*                  m_connect_menu;
    QMenu*                  m_disconnect_menu;

    QAction*                m_connect_action;
    QAction*                m_disconnect_action;
    QAction*                m_remove_action;
    QAction*                m_remove_all_action;

    void switchAddBtn();
public:
    explicit servers_widget(QWidget *parent = 0);
    ~servers_widget();
private slots:
    void serversSortChanged(int, Qt::SortOrder);
    void on_btnAdd_clicked();
    void on_editIP_textChanged(const QString &arg1);
    void on_editName_textChanged(const QString &arg1);
    void displayHeaderMenu(const QPoint&);

    void connect_slot();
    void disconnect_slot();
    void remove_slot();
    void removeAll_slot();
    void on_tableServers_customContextMenuRequested(const QPoint &pos);
};

#endif // SERVERS_WIDGET_H
