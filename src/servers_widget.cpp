#include "servers_widget.h"

servers_widget::servers_widget(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    QList<int> sz;
    sz << 500 << 150;
    vsplit->setSizes(sz);
}

servers_widget::~servers_widget()
{
}
