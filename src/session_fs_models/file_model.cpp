#include "file_model.h"
#include <QFont>
#include <QBrush>

FilesModel::FilesModel(DirNode* root, QObject* parent /*= 0*/) : BaseModel(root, parent)
{
}

QModelIndex FilesModel::index(int row, int column,
                  const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
            return QModelIndex();

    DirNode* parentNode;

    if (!parent.isValid())
        parentNode = m_rootItem;
    else
        parentNode = static_cast<DirNode*>(parent.internalPointer());
    Q_ASSERT(row < parentNode->m_file_children.size());
    FileNode *childNode = parentNode->m_file_vector.at(row);
    Q_ASSERT(childNode);
    return createIndex(row, column, childNode);
}

int FilesModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    DirNode* parentNode;

    if (!parent.isValid())
        parentNode = m_rootItem;
    else
        parentNode = static_cast<DirNode*>(parent.internalPointer());

    parentNode->populate();
    return parentNode->m_file_vector.size();
}

bool FilesModel::hasChildren(const QModelIndex & parent /*= QModelIndex()*/) const
{
    bool res = false;
    if (!parent.isValid()) res = true;
    return res;
}

QModelIndex FilesModel::index(const FileNode* node)
{
    DirNode *parentNode = (node ? node->m_parent : 0);

    if (node == m_rootItem || !parentNode || node->is_dir())
        return QModelIndex();

    // get the parent's row
    Q_ASSERT(node);

    int row = 0;
    // search node
    foreach(const FileNode* p, parentNode->m_file_vector)
    {
        if (p == node)
        {
            break;
        }

        ++row;
    }

    return createIndex(row, 0, const_cast<FileNode*>(node));
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation,
                    int role /*= Qt::DisplayRole*/) const
{
    switch (role)
    {
    case Qt::DecorationRole:
        if (section == 0)
        {
            // ### TODO oh man this is ugly and doesn't even work all the way!
            // it is still 2 pixels off
            QImage pixmap(16, 1, QImage::Format_Mono);
            pixmap.fill(0);
            pixmap.setAlphaChannel(pixmap.createAlphaMask());
            return pixmap;
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractItemModel::headerData(section, orientation, role);

    QString returnValue;
    switch (section)
    {
    case DC_STATUS:
                    returnValue = "";
            break;
    case DC_NAME:   returnValue = FilesModel::tr("Name");
            break;
    case DC_FSIZE:   returnValue = FilesModel::tr("Size");
            break;
    case DC_TYPE:   returnValue =
#ifdef Q_OS_MAC
                   FilesModel::tr("Kind", "Match OS X Finder");
#else
                   FilesModel::tr("Type", "All other platforms");
#endif
           break;
    // Windows   - Type
    // OS X      - Kind
    // Konqueror - File Type
    // Nautilus  - Type
    case DC_TIME:   returnValue = FilesModel::tr("Date Modified");
            break;
    case DC_HASH:   returnValue = FilesModel::tr("Transfer hash");
            break;
    case DC_ERROR:  returnValue = FilesModel::tr("Transfer state");
            break;
    default: return QVariant();
    }

    return returnValue;
}

Qt::ItemFlags FilesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
            return 0;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == DC_STATUS) flags |=  Qt::ItemIsUserCheckable;
    return flags;
}

QVariant FilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case BaseModel::SortRole:
        switch(index.column())
        {
            case DC_STATUS: return state(index);
            case DC_NAME:   return displayName(index);
            case DC_FSIZE:   return size(index);
            case DC_TYPE:   return type(index);
            case DC_TIME:   return dt(index);
            case DC_HASH:   return hash(index);
            case DC_ERROR:  return has_error(index);
            default:
                qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case Qt::CheckStateRole:
        {
            if (index.column() == DC_STATUS)
            {
                return state(index);
            }
        }
    break;
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column())
        {
            case DC_STATUS: return QVariant();
            case DC_NAME:   return displayName(index);
            case DC_FSIZE:  return size(size(index));
            case DC_TYPE:   return type(index);
            case DC_TIME:   return time(index);
            case DC_HASH:   return hash(index);
            case DC_ERROR:  return error(index);
            default:
                qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return filepath(index);
    case FileNameRole:
        return name(index);
    case Qt::DecorationRole:
        if (index.column() == DC_NAME)
        {
            QIcon icon = this->icon(index);

            if (icon.isNull())
            {
                if (node(index)->is_dir())
                    icon = m_iconProvider.icon(QFileIconProvider::Folder);
                else
                    icon = m_iconProvider.icon(QFileIconProvider::File);
            }
            return icon;
        }

        break;
    case Qt::TextAlignmentRole:
        if (index.column() == DC_FSIZE)
            return (Qt::AlignRight | Qt::AlignVCenter);
        break;
    case Qt::FontRole:
        if (index.column() == DC_NAME)
        {
            if (this->contains_active_children(index))
            {
                QFont f;
                f.setBold(true);
                return f;
            }
        }
    case Qt::BackgroundRole:
        if (!error(index).isEmpty())
        {
            return QBrush(Qt::yellow);
        }
    case FilePermissions:
        int p = permissions(index);
        return p;
    }

    return QVariant();
}

bool FilesModel::setData ( const QModelIndex & index, const QVariant & value, int role/* = Qt::EditRole*/)
{
    bool res = false;

    if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
    {
        FileNode* node = static_cast<FileNode*>(index.internalPointer());
        Q_ASSERT(node);
        qDebug() << "node filepath " << node->filepath();

        if (node->is_active())
        {
            node->unshare(false);
        }
        else
        {
            node->share(false);
        }

        res = true;
    }

    return res;
}

QModelIndex FilesModel::node2index(const FileNode* node) const
{
    Q_ASSERT(node);
    if (node == m_rootItem || node->is_dir())
        return QModelIndex();
    return createIndex(node2row(node), 0, const_cast<FileNode*>(node));
}

int FilesModel::node2row(const FileNode* node) const
{
    int row = -1;

    if (node != m_rootItem)
    {
        row = 0;
        foreach(const FileNode* p, node->m_parent->m_file_vector)
        {
            if (p == node) break;
            ++row;
        }
    }

    return row;
}

void FilesModel::emitChangeSignal(const QModelIndex& indx)
{
    emit dataChanged(indx, indx);
    emit dataChanged(index(indx.row(), columnCount() - 2, parent(indx)),
                               index(indx.row(), columnCount() - 1, parent(indx)));
}

