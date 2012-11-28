#include "path_model.h"
#include "session.h"

PathModel::PathModel(QObject *parent /* = 0*/)
    : QAbstractListModel(parent),
      m_all_icon(":/emule/files/all.ico"),
      m_sd_icon(":/emule/common/folder_share.ico")
{
    foreach(const DirNode* node, Session::instance()->directories())
    {
        m_paths.append(node);
    }

    connect(Session::instance(), SIGNAL(insertSharedDirectory(const DirNode*)), this, SLOT(on_insertSharedDirectory(const DirNode*)));
    connect(Session::instance(), SIGNAL(removeSharedDirectory(const DirNode*)), this, SLOT(on_removeSharedDirectory(const DirNode*)));
}

int PathModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return m_paths.size() + 1;
}

QVariant PathModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() > m_paths.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        if (index.row() == 0)
        {
            return tr("All shared files");
        }

        return m_paths.at(index.row() - 1)->filepath();
    }
    else if (role == Qt::DecorationRole)
    {
        if (index.row() == 0)
        {
            return m_all_icon;
        }

        return m_sd_icon;
    }

    return QVariant();
}

QVariant PathModel::headerData(int section, Qt::Orientation orientation,
                    int role /*= Qt::DisplayRole*/) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Filepath");

    return QVariant();
}

const DirNode* PathModel::node(const QModelIndex& indx) const
{
    const DirNode* res = Session::instance()->root();

    if (indx.isValid())
    {
        if (indx.row() != 0)
        {
            res = m_paths.at(indx.row() - 1);            
        }
    }

    return res;
}

const QString PathModel::filepath(const QModelIndex& indx) const
{
    QString res;

    if (indx.isValid())
    {
        if (indx.row() != 0)
        {
            res = m_paths.at(indx.row() - 1)->filepath();
        }
    }

    return res;
}

void PathModel::on_removeSharedDirectory(const DirNode* node)
{
    int row = node2row(node);

    if (row != -1)
    {
        beginRemoveRows(QModelIndex(), row, row);
        m_paths.removeAt(row);
        endRemoveRows();
    }
}

void PathModel::on_insertSharedDirectory(const DirNode* node)
{
    if (node2row(node) == -1)
    {
        beginInsertRows(QModelIndex(), m_paths.size(), m_paths.size());
        m_paths.append(node);
        endInsertRows();
    }
}

int PathModel::node2row(const DirNode* node) const
{
    int row = -1;

    foreach(const DirNode* p, m_paths)
    {
        ++row;
        if (node == p)
            return row;
    }

    return -1;
}
