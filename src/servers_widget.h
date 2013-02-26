#ifndef SERVERS_WIDGET_H
#define SERVERS_WIDGET_H

#include <QWidget>
#include "ui_servers_widget.h"

class servers_widget : public QWidget, private Ui::servers_widget
{
    Q_OBJECT
    
public:
    explicit servers_widget(QWidget *parent = 0);
    ~servers_widget();
};

#endif // SERVERS_WIDGET_H
