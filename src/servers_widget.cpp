#include "servers_widget.h"
#include "preferences.h"
#include "servers_table_model.h"

servers_widget::servers_widget(QWidget *parent) :
    QWidget(parent), m_smodel(NULL)
{
    setupUi(this);

    m_smodel = new servers_table_model(this);
    m_smodel->load();
    tableServers->setModel(m_smodel);

    Preferences pref;

    if (!hsplit->restoreState(pref.value("ServersWidget/HSplitter").toByteArray()))
    {
        QList<int> sz;
        sz << 500 << 100;
        hsplit->setSizes(sz);
    }

    vsplit->restoreState(pref.value("ServersWidget/VSplitter").toByteArray());
    //tableServers->horizontalHeader()->restoreState(pref.value("ServersWidget/TableServers").toByteArray());
}

servers_widget::~servers_widget()
{
    Preferences pref;
    pref.setValue("ServersWidget/VSplitter", vsplit->saveState());
    pref.setValue("ServersWidget/HSplitter", hsplit->saveState());
    pref.setValue("ServersWidget/TableServers", tableServers->horizontalHeader()->saveState());
}
