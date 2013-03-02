#ifndef SERVERS_WIDGET_H
#define SERVERS_WIDGET_H

#include <QWidget>
#include "ui_servers_widget.h"

class servers_table_model;
class QSortFilterProxyModel;
class QMenu;
class QAction;
class QHostInfo;

class servers_widget : public QWidget, private Ui::servers_widget
{
    Q_OBJECT
private:
    QSortFilterProxyModel*  m_sort_model;
    servers_table_model*    m_smodel;
    QMenu*                  m_connect_menu;
    QAction*                m_connect_action;
    QAction*                m_remove_action;
    QAction*                m_remove_all_action;

    QString last_server_name;
    int last_server_port;
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

    void connect_handler();
    void remove_handler();
    void removeAll_handler();
    void on_tableServers_customContextMenuRequested(const QPoint &pos);
    void lookedUP(const QHostInfo&);
};

#endif // SERVERS_WIDGET_H
