#include "shared_files_model.h"

SFModel::SFModel(QObject *parent /*= 0*/) : FilesModel(Session::instance()->root(), parent)
{
    sync();
    connect(Session::instance(), SIGNAL(removeSharedFile(FileNode*)), this, SLOT(on_removeSharedFile(FileNode*)));
    connect(Session::instance(), SIGNAL(insertSharedFile(FileNode*)), this, SLOT(on_insertSharedFile(FileNode*)));
}

int SFModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return m_files.size();
}

QModelIndex SFModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

QModelIndex SFModel::index(int row, int column,
                  const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
            return QModelIndex();

    DirNode* parentNode;

    if (!parent.isValid())
        parentNode = m_rootItem;
    else
        parentNode = static_cast<DirNode*>(parent.internalPointer());

    Q_ASSERT(row < rowCount(parent));

    FileNode *childNode = m_files.at(row);
    Q_ASSERT(childNode);
    return createIndex(row, column, childNode);
}

void SFModel::setFilter(BaseFilter* filter)
{
    m_filter.reset(filter);
    m_files.clear();
    sync();
    reset();
}

int SFModel::node2row(const FileNode* node) const
{
    int row = -1;

    foreach(FileNode* p, m_files)
    {
        ++row;
        if (node == p)
            return row;
    }

    return -1;
}

void SFModel::sync()
{
    foreach(FileNode* p, Session::instance()->files().values())
    {
        if (!m_filter.isNull() && (m_filter->match(p->parent_path(), p->filename())))
            m_files.append(p);
    }
}

void SFModel::on_removeSharedFile(FileNode* node)
{
    int row = node2row(node);

    if (row != -1)
    {
        beginRemoveRows(QModelIndex(), row, row);
        m_files.removeAt(row);
        endRemoveRows();
    }
}

void SFModel::on_insertSharedFile(FileNode* node)
{
    if (!m_filter.isNull() && (m_filter->match(node->parent_path(), node->filename())))
    {
        beginInsertRows(QModelIndex(), m_files.size(), m_files.size());
        m_files.append(node);
        endInsertRows();
    }
}
