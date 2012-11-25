#include "dir_model.h"
#include <QPainter>
#include <QPixmap>

DirectoryModel::DirectoryModel(DirNode* root, QObject* parent /*= 0*/) : BaseModel(root, parent)
{

}

QModelIndex DirectoryModel::index(int row, int column,
                  const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
            return QModelIndex();

    DirNode* parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<DirNode*>(parent.internalPointer());

    Q_ASSERT(row < parentItem->m_dir_vector.size());
    DirNode *childItem = parentItem->m_dir_vector.at(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

int DirectoryModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    DirNode* parentNode;

    if (!parent.isValid())
        parentNode = m_rootItem;
    else
        parentNode = static_cast<DirNode*>(parent.internalPointer());

    parentNode->populate();
    return parentNode->m_dir_vector.size();
}

bool DirectoryModel::hasChildren(const QModelIndex & parent /* = QModelIndex()*/) const
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

            if (!pd->is_populated() || !pd->m_dir_vector.empty())
            {
                res = true;
            }
        }
    }

    return res;
}

QModelIndex DirectoryModel::node2index(const FileNode* node) const
{
    Q_ASSERT(node);

    if (node == m_rootItem || !node->is_dir())
        return QModelIndex();

    return createIndex(node2row(node), 0, const_cast<FileNode*>(node));
}

int DirectoryModel::node2row(const FileNode* node) const
{
    int row = -1;

    if (node != m_rootItem)
    {
        row = 0;
        foreach(const DirNode* p, node->m_parent->m_dir_vector)
        {
            if (p == node) break;
            ++row;
        }
    }

    return row;
}


QVariant DirectoryModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    // TODO - implement orientation
    QVariant res;

    if (section == DC_STATUS && role == Qt::DisplayRole)
    {
        res = tr("Name");
    }

    return (res);
}

Qt::ItemFlags DirectoryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
            return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return flags;
}

QVariant DirectoryModel::data(const QModelIndex &index, int role) const
{
    QVariant res;

    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        if (index.column() == DC_STATUS)
        {
            res = displayName(index);
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == DC_STATUS)
        {
            QIcon icon = this->icon(index);

            if (icon.isNull())
            {
                if (node(index)->is_dir())
                    icon = m_iconProvider.icon(QFileIconProvider::Folder);
                else
                    icon = m_iconProvider.icon(QFileIconProvider::File);
            }

            if (this->active(index))
            {
                // prepare mule head
                QPixmap pixm = icon.pixmap(icon.availableSizes()[0]);
                QPainter painter(&pixm);
                painter.drawPixmap(0, 0, QPixmap(":emule/files/SharedFolderOvl.png"));
                painter.end();
                res = QIcon(pixm);
            }
            else
            {
                res = icon;
            }
        }

        break;
    case Qt::FontRole:
        if (index.column() == DC_STATUS)
        {
            if (this->contains_active_children(index))
            {
                QFont f;
                f.setBold(true);
                res = f;
            }
        }
    default:
        break;
    }

    return res;
}

void DirectoryModel::emitChangeSignal(const QModelIndex& indx)
{
    emit dataChanged(indx, indx);
}
