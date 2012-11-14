#include "share_files.h"
#include "tree.h"
#include <QFont>
#include <QPainter>
#include <QPixmap>

TreeModel::TreeModel(DirNode* root, Filter filter, QObject *parent) : QAbstractItemModel(parent), m_rootItem(root), m_filter(filter)
{
    m_change = false;
}

TreeModel::~TreeModel()
{
}

bool TreeModel::setData ( const QModelIndex & index, const QVariant & value, int role/* = Qt::EditRole*/)
{
    bool res = false;

    if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
    {
        qDebug() << "execute setData " << value;
        FileNode* node = static_cast<FileNode*>(index.internalPointer());
        Q_ASSERT(node);

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

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::CheckStateRole:
        {
            if (index.column() == DC_STATUS) return active(index);
        }
    break;
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column())
        {
        case DC_STATUS: return QVariant();
            case DC_NAME:   return displayName(index);
            case DC_SIZE:   return size(index);
            case DC_TYPE:   return type(index);
            case DC_TIME:   return time(index);
            case DC_HASH:   return hash(index);
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
        if (index.column() == DC_SIZE)
            return Qt::AlignRight;
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
    case FilePermissions:
        int p = permissions(index);
        return p;
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
            return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == DC_STATUS) flags |=  Qt::ItemIsUserCheckable;
    return flags;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
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
    case DC_STATUS: returnValue = tr("Status");
            break;
    case DC_NAME:   returnValue = tr("Name");
            break;
    case DC_SIZE:   returnValue = tr("Size");
            break;
    case DC_TYPE:   returnValue =
#ifdef Q_OS_MAC
                   tr("Kind", "Match OS X Finder");
#else
                   tr("Type", "All other platforms");
#endif
           break;
    // Windows   - Type
    // OS X      - Kind
    // Konqueror - File Type
    // Nautilus  - Type
    case DC_TIME:   returnValue = tr("Date Modified");
            break;
    case DC_HASH:   returnValue = tr("Transfer hash");
            break;
    default: return QVariant();
    }

    return returnValue;
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


    Q_ASSERT(row < elements_count(parentItem));

    FileNode *childItem = NULL;

    if (m_filter == TreeModel::All)
    {
        if (row < parentItem->m_dir_children.size())
        {
            childItem = parentItem->m_dir_vector.at(row);
        }
        else
        {
            childItem = parentItem->m_file_vector.at(row - parentItem->m_dir_children.size());
        }
    }
    else if (m_filter == TreeModel::Dir)
    {
        childItem = parentItem->m_dir_vector.at(row);
    }
    else
    {
        childItem = parentItem->m_file_vector.at(row);
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
    return elements_count(parentItem);
}

int TreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return (parent.column() > 0) ? 0 : 6;
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

            if (!pd->is_populated() ||
                    (!pd->m_file_vector.empty() && (m_filter & TreeModel::File)) ||
                    (!pd->m_dir_vector.empty() && (m_filter & TreeModel::Dir)))
            {
                res = true;
            }
        }
    }

    return res;
}

int TreeModel::elements_count(const DirNode* node) const
{
    int res = 0;

    if (node)
    {
        if (m_filter & TreeModel::Dir)
        {
            res += node->m_dir_vector.size();
        }

        if (m_filter & TreeModel::File)
        {
            res += node->m_file_vector.size();
        }
    }

    return res;
}

void TreeModel::setRootNode(const QModelIndex& index)
{
    if (!index.isValid()) return;
    qDebug() << "set index to " << static_cast<DirNode*>(index.internalPointer())->filepath();
    m_rootItem = static_cast<DirNode*>(index.internalPointer());
    reset();
}

QModelIndex TreeModel::index(const FileNode* node)
{

    DirNode *parentNode = (node ? node->m_parent : 0);

    if (node == m_rootItem || !parentNode)
        return QModelIndex();

    // get the parent's row
    Q_ASSERT(node);

    // filter out unnesessary nodes
    if (m_filter != TreeModel::All)
    {
        if ((m_filter == TreeModel::Dir) && !node->is_dir()) return QModelIndex();
        if ((m_filter == TreeModel::File) && node->is_dir()) return QModelIndex();
    }

    int row = 0;
    // search node
    if (node->is_dir())
    {
        foreach(const DirNode* p, parentNode->m_dir_vector)
        {
            if (p == node)
            {
                break;
            }
            ++row;
        }
    }
    else
    {
        foreach(const FileNode* p, parentNode->m_file_vector)
        {
            if (p == node)
            {
                break;
            }
            ++row;
        }

        // correct row when all types requested
        if (m_filter == TreeModel::All)
        {
            row += parentNode->m_dir_vector.size();
        }
    }

    qDebug() << "generate index " << row;
    return createIndex(row, 0, const_cast<FileNode*>(node));
}

// helper functions
QString TreeModel::hash(const QModelIndex& index) const
{
    if (!index.isValid()) return QString();
    return node(index)->hash();
}

bool TreeModel::contains_active_children(const QModelIndex& index) const
{
    if (!index.isValid()) return 0;
    return node(index)->contains_active_children();
}

bool TreeModel::active(const QModelIndex& index) const
{
    if (!index.isValid()) return false;
    return node(index)->is_active();
}

qint64 TreeModel::size(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;
    return node(index)->size_on_disk();
}

QString TreeModel::type(const QModelIndex &index) const
{
    if (!index.isValid()) return QString();
    FileNode* p = node(index);

    if (p->m_displayType.isEmpty())
    {
        p->m_displayType = m_iconProvider.type(p->m_info);
    }

    return p->m_displayType;
}

QDateTime TreeModel::lastModified(const QModelIndex &index) const
{
    if (!index.isValid()) return QDateTime();
    return node(index)->m_info.lastModified();
}

QIcon TreeModel::icon(const QModelIndex& index) const
{
    if (!index.isValid()) return QIcon();
    FileNode* p = node(index);

    if (p->m_icon.isNull())
    {
        p->m_icon = m_iconProvider.icon(p->m_info);
    }

    return p->m_icon;
}

QString TreeModel::displayName(const QModelIndex &index) const
{
    return name(index);
}

QString TreeModel::name(const QModelIndex& index) const
{
    if (!index.isValid()) return QString();
    return node(index)->filename();
}

QString TreeModel::filepath(const QModelIndex& index) const
{
    if (!index.isValid()) return QString();
    return node(index)->filepath();
}

QString TreeModel::time(const QModelIndex& index) const
{
    if (!index.isValid()) return QString();
#ifndef QT_NO_DATESTRING
    return node(index)->m_info.lastModified().toString(Qt::SystemLocaleDate);
#else
    Q_UNUSED(index);
    return QString();
#endif
}

QFile::Permissions TreeModel::permissions(const QModelIndex &index) const
{
    if (!index.isValid()) return QFile::Permissions();
    return node(index)->m_info.permissions();
}

QString TreeModel::size(qint64 bytes) const
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if (bytes >= tb)
        return tr("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
    if (bytes >= gb)
        return tr("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return tr("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
    if (bytes >= kb)
        return tr("%1 KB").arg(QLocale().toString(bytes / kb));
    return tr("%1 bytes").arg(QLocale().toString(bytes));
}

FileNode* TreeModel::node(const QModelIndex& index) const
{
    FileNode* p = static_cast<FileNode*>(index.internalPointer());
    Q_ASSERT(p);
    return (p);
}

void TreeModel::removeNode(const FileNode* node)
{
    QModelIndex indx = index(node);

    if (indx.isValid())
    {
        QModelIndex parentIndx = parent(indx);
        if (parentIndx.isValid()) removeRow(indx.row(), parentIndx);
    }
}

void TreeModel::addNode(const FileNode* node)
{
    QModelIndex indx = index(node);

    if (indx.isValid())
    {
        QModelIndex parentindx = parent(indx);
        if (parentindx.isValid()) insertRow(indx.row(), parentindx);
    }
}

void TreeModel::changeNode(const FileNode* node)
{
    QModelIndex indx = index(node);    
    if (indx.isValid()) emit dataChanged(indx, sibling(indx.row(), columnCount() - 1, indx));
}

void TreeModel::beginRemoveNode(const FileNode* node)
{    
    QModelIndex indx = index(node);

    m_change = false;
    if (indx.isValid())
    {
        int row = 0;
        FileNode* node = static_cast<FileNode*>(indx.internalPointer());
        DirNode* parent_node = node->m_parent;

        if (node->is_dir())
        {
            foreach(DirNode* p, parent_node->m_dir_vector)
            {
                if (p == node) break;
                ++row;
            }
        }
        else
        {
            foreach(FileNode* p, parent_node->m_file_vector)
            {
                if (p == node) break;
                ++row;
            }
        }

        qDebug() << "beginRemoveNode row: " << row;
        beginRemoveRows(parent(indx), row, row);
        m_change = true;
    }
}

void TreeModel::endRemoveNode()
{
    if (m_change) endRemoveRows();
    m_change = false;
}

void TreeModel::beginInsertNode(const FileNode* node, int pos)
{    
    QModelIndex indx = index(node);
    m_change = false;

    if (indx.isValid())
    {
        int row = 0;
        qDebug() << "index is valid";
        FileNode* node = static_cast<FileNode*>(indx.internalPointer());
        DirNode* parent_node = node->m_parent;

        /*
        if (node->is_dir())
        {
            row += parent_node->m_dir_vector.size();
        }
        else
        {
            row += parent_node->m_file_vector.size();
        }
*/
        qDebug() << "beginInsertNode row: " << pos;
        beginInsertRows(index(parent_node), pos, pos);
        m_change = true;
    }
}

void TreeModel::endInsertNode()
{
    if (m_change) endInsertRows();
    m_change = false;
}

QVariant DirModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    QVariant res;

    if (section == DC_STATUS && role == Qt::DisplayRole)
    {
        res = tr("Name");
    }

    return (res);
}

QVariant DirModel::data(const QModelIndex &index, int role) const
{
    QVariant res;

    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::EditRole:
        break;
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
                painter.drawPixmap(0, 0, QPixmap("../../src/Icons/emule/files/SharedFolderOvl.png"));
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
