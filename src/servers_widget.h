#ifndef SERVERS_WIDGET_H
#define SERVERS_WIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include "ui_servers_widget.h"

class servers_table_model;

class servers_widget : public QWidget, private Ui::servers_widget
{
    Q_OBJECT
private:
    QSortFilterProxyModel*  m_sort_model;
    servers_table_model*    m_smodel;

    void displayHeaderMenu(const QPoint&);
    void switchAddBtn();
public:
    explicit servers_widget(QWidget *parent = 0);
    ~servers_widget();
private slots:
    void serversSortChanged(int, Qt::SortOrder);
    void on_btnAdd_clicked();
    void on_editIP_textChanged(const QString &arg1);
    void on_editName_textChanged(const QString &arg1);
};

#endif // SERVERS_WIDGET_H
