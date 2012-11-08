#include "share_files.h"
#include "tree.h"

TreeModel::TreeModel(DirNode* root, QObject *parent) : QAbstractItemModel(parent), m_rootItem(root)
{

}

TreeModel::~TreeModel()
{

}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    FileNode* item = static_cast<FileNode*>(index.internalPointer());

    qDebug() << "data " << item->filename();
    return item->filename();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
            return 0;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                    int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant("filename");

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column,
                  const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
            return QModelIndex();

    DirNode* parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<DirNode*>(parent.internalPointer());

    Q_ASSERT(row < (parentItem->m_dir_children.size() + parentItem->m_file_children.size()));

    FileNode *childItem = NULL;

    if (row < parentItem->m_dir_children.size())
    {
        childItem = parentItem->m_dir_vector.at(row);
    }
    else
    {
        childItem = parentItem->m_file_vector.at(row - parentItem->m_dir_children.size());
    }

    Q_ASSERT(childItem);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
            return QModelIndex();

    FileNode* childItem = static_cast<FileNode*>(index.internalPointer());
    DirNode* parentItem = childItem->m_parent;

    if (parentItem == m_rootItem)
            return QModelIndex();

    int row = 0;

    if (parentItem->m_parent)
    {
        foreach(const DirNode* p, parentItem->m_dir_vector)
        {
            if (p == parentItem)
            {
                break;
            }

            ++row;
        }
    }

    return createIndex(row, 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    DirNode* parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<DirNode*>(parent.internalPointer());

    parentItem->populate();
    qDebug() << "rowCount is " << (parentItem->m_file_vector.size() + parentItem->m_dir_vector.size());
    return (parentItem->m_file_vector.size() + parentItem->m_dir_vector.size());
}

int TreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 1;
    //if (parent.isValid())
    //         return static_cast<FileNode*>(parent.internalPointer())->columnCount();
   //
   //      else
   //          return rootItem->columnCount();
}

bool TreeModel::hasChildren(const QModelIndex & parent /* = QModelIndex()*/) const
{
    bool res = false;
    if (!parent.isValid())
    {
        res = true;
    }
    else
    {
        const FileNode* p = static_cast<const FileNode*>(parent.internalPointer());

        if (p->is_dir())
        {
            DirNode* pd = (DirNode*)p;
            if (!pd->is_populated() || (!pd->m_file_vector.empty() || !pd->m_dir_vector.empty()))
            {
                res = true;
            }
        }
    }

    return res;
}
