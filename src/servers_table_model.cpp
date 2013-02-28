#include <fstream>
#include <libed2k/error_code.hpp>
#include "servers_table_model.h"
#include "misc.h"

#include <QDebug>

servers_table_model::servers_table_model(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int servers_table_model::rowCount(const QModelIndex& parent) const
{
    return 2;
}

int servers_table_model::columnCount(const QModelIndex& parent) const
{
    return server_met.m_servers.count();
}

QVariant servers_table_model::data(const QModelIndex& index, int role) const
{
    QVariant res;

    if (index.isValid() && (index.row() < rowCount(QModelIndex())) && (index.column() < columnCount(QModelIndex())))
    {
        // assign res
    }

    return res;
}

QVariant servers_table_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    return QVariant();
}

void servers_table_model::save()
{
    try
    {
        std::ofstream fs(misc::ED2KMetaLocation("server.met").toLocal8Bit(), std::ios_base::out | std::ios_base::binary);

        if (fs)
        {
            libed2k::archive::ed2k_oarchive oa(fs);
            oa << server_met;
        }
    }
    catch(libed2k::libed2k_exception& e)
    {
        qDebug() << "error on store servers model " << e.what();
    }
}

void servers_table_model::load()
{
    try
    {
        std::ifstream fs(misc::ED2KMetaLocation("server.met").toLocal8Bit(), std::ios_base::out | std::ios_base::binary);

        if (fs)
        {
            libed2k::archive::ed2k_iarchive ia(fs);
            ia >> server_met;
        }
    }
    catch(libed2k::libed2k_exception& e)
    {
        qDebug() << "error on load servers model " << e.what();
    }
}
