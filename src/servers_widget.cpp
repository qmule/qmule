#include <QMenu>
#include "servers_widget.h"
#include "preferences.h"
#include "servers_table_model.h"

servers_widget::servers_widget(QWidget *parent) :
    QWidget(parent), m_smodel(NULL)
{
    setupUi(this);

    m_smodel = new servers_table_model(this);
    m_smodel->load();

    m_sort_model = new QSortFilterProxyModel(this);
    m_sort_model->setSourceModel(m_smodel);
    tableServers->setModel(m_sort_model);

    Preferences pref;

    if (!hsplit->restoreState(pref.value("ServersWidget/HSplitter").toByteArray()))
    {
        QList<int> sz;
        sz << 500 << 100;
        hsplit->setSizes(sz);
    }

    vsplit->restoreState(pref.value("ServersWidget/VSplitter").toByteArray());
    //tableServers->horizontalHeader()->restoreState(pref.value("ServersWidget/TableServers").toByteArray());

    connect(tableServers->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(serversSortChanged(int, Qt::SortOrder)));

    connect(tableServers->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(displayHeaderMenu(const QPoint&)));

    tableServers->horizontalHeader()->setSortIndicator(servers_table_model::DC_NAME, Qt::AscendingOrder);
}

servers_widget::~servers_widget()
{
    Preferences pref;
    pref.setValue("ServersWidget/VSplitter", vsplit->saveState());
    pref.setValue("ServersWidget/HSplitter", hsplit->saveState());
    pref.setValue("ServersWidget/TableServers", tableServers->horizontalHeader()->saveState());
}

void servers_widget::serversSortChanged(int column, Qt::SortOrder order)
{
    m_sort_model->sort(column, order);
}

void servers_widget::displayHeaderMenu(const QPoint&)
{
    QMenu hideshowColumn(this);
    hideshowColumn.setTitle(tr("Column visibility"));
    QList<QAction*> actions;

    for (int i=0; i < m_smodel->columnCount(); ++i)
    {
        QAction *myAct = hideshowColumn.addAction(
            m_smodel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        myAct->setCheckable(true);
        myAct->setChecked(!tableServers->isColumnHidden(i));
        actions.append(myAct);
    }

    // Call menu
    QAction *act = hideshowColumn.exec(QCursor::pos());

    if (act)
    {
        int col = actions.indexOf(act);
        Q_ASSERT(col >= 0);
        tableServers->setColumnHidden(col, !tableServers->isColumnHidden(col));
        if (!tableServers->isColumnHidden(col) && tableServers->columnWidth(col) <= 5)
            tableServers->setColumnWidth(col, 100);
    }
}

void servers_widget::on_btnAdd_clicked()
{
    // add new server
    editName->clear();
    editIP->clear();
}

void servers_widget::on_editIP_textChanged(const QString &arg1)
{
    switchAddBtn();
}

void servers_widget::on_editName_textChanged(const QString &arg1)
{
    switchAddBtn();
}

void servers_widget::switchAddBtn()
{
    if (!editName->text().isEmpty() && !editIP->text().isEmpty())
    {
        btnAdd->setEnabled(true);
    }
    else
    {
        btnAdd->setEnabled(false);
    }
}

