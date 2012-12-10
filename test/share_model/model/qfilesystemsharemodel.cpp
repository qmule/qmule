#include "qfilesystemsharemodel.h"

QFileSystemShareModel::QFileSystemShareModel(shared_files_tree* pSFTree) : m_pSFTree(pSFTree)
{
}

QFileSystemShareModel::~QFileSystemShareModel()
{
    delete m_pSFTree;
}

shared_files_tree::QFileSystemNode* QFileSystemShareModel::node(const QModelIndex& index) const
{
    shared_files_tree::QFileSystemNode* p = static_cast<shared_files_tree::QFileSystemNode*>(index.internalPointer());
    Q_ASSERT(p);
    return (p);
}

QModelIndex QFileSystemShareModel::index(const shared_files_tree::QFileSystemNode* node) const
{
    shared_files_tree::QFileSystemNode *parentNode = (node ? node->parent : 0);
    if (node == m_pSFTree->rootNode() || !parentNode)
        return QModelIndex();
    // get the parent's row
    Q_ASSERT(node);
    return createIndex(node->m_order, 0, const_cast<shared_files_tree::QFileSystemNode*>(node));
}

QVariant QFileSystemShareModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column())
        {
        case DC_NAME: return displayName(index);
        case DC_SIZE: return size(index);
        case DC_TYPE: return type(index);
        case DC_TIME: return time(index);
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return filePath(index);
    case FileNameRole:
        return name(index);
    case Qt::DecorationRole:
        if (index.column() == 0)
        {
            QIcon icon = this->icon(index);

            if (icon.isNull())
            {
                if (node(index)->isDir())
                    icon = m_pSFTree->m_fileinfo_gatherer.iconProvider()->icon(QFileIconProvider::Folder);
                else
                    icon = m_pSFTree->m_fileinfo_gatherer.iconProvider()->icon(QFileIconProvider::File);
            }
            return icon;
        }

        break;
    case Qt::TextAlignmentRole:
        if (index.column() == 1)
            return Qt::AlignRight;
        break;
    case FilePermissions:
        int p = permissions(index);
        return p;
    }

    return QVariant();
}

Qt::ItemFlags QFileSystemShareModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

QVariant QFileSystemShareModel::headerData(int section, Qt::Orientation orientation,
                         int role /*Qt::DisplayRole*/) const
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
    case DC_NAME: returnValue = tr("Name");
            break;
    case DC_SIZE: returnValue = tr("Size");
            break;
    case DC_TYPE: returnValue =
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
    case DC_TIME: returnValue = tr("Date Modified");
            break;
    default: return QVariant();
    }

    return returnValue;
}

QModelIndex QFileSystemShareModel::index(int row, int column,
           const QModelIndex &parent /* QModelIndex()*/) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
     return QModelIndex();

    shared_files_tree::QFileSystemNode* parent_node;

    if (!parent.isValid())
    {
        parent_node = m_pSFTree->rootNode();
    }
    else
    {
        parent_node = node(parent);
    }

    Q_ASSERT(parent_node);

    shared_files_tree::QFileSystemNode* node = m_pSFTree->node(parent_node, row);
    Q_ASSERT(node);
    return createIndex(row, column, node);
}

QModelIndex QFileSystemShareModel::index(const QString& path, int column /*= 0*/) const
{
    // TODO - implement if need
    Q_UNUSED(path);
    Q_UNUSED(column);
    return (QModelIndex());
}

QModelIndex QFileSystemShareModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())  return QModelIndex();

    shared_files_tree::QFileSystemNode* node = static_cast<shared_files_tree::QFileSystemNode*>(index.internalPointer());
    shared_files_tree::QFileSystemNode* parent_node = node->parent;

    if (!parent_node || parent_node == m_pSFTree->rootNode())
    {
        return QModelIndex();
    }

    return createIndex(parent_node->m_order, 0, parent_node);
}

bool QFileSystemShareModel::canFetchMore(const QModelIndex &parent) const
{
    return (!node(parent)->populatedChildren);
}

void QFileSystemShareModel::fetchMore(const QModelIndex &parent)
{
    shared_files_tree::QFileSystemNode* p = node(parent);
    if (p->populatedChildren) return;
    p->populatedChildren = true;
}

int QFileSystemShareModel::rowCount(const QModelIndex &parent /* QModelIndex()*/) const
{
    if (parent.column() > 0) return 0;

    shared_files_tree::QFileSystemNode* parent_node;

    if (!parent.isValid())
    {
        parent_node = m_pSFTree->rootNode();
    }
    else
    {
        parent_node = node(parent);
    }

    Q_ASSERT(parent_node);

    return parent_node->children.count();
}

int QFileSystemShareModel::columnCount(const QModelIndex &parent /* = QModelIndex()*/) const
{
    return (parent.column() > 0) ? 0 : 4;
}

bool QFileSystemShareModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) return true;
    return (node(parent)->isDir());
}

QString QFileSystemShareModel::filePath(const QModelIndex &index) const
{
    QString path;

    if (index.isValid())
    {
        path = m_pSFTree->filePath(node(index));
    }

    return path;
}

bool QFileSystemShareModel::isDir(const QModelIndex &index) const
{
    bool res = true;

    if (index.isValid())
    {
        shared_files_tree::QFileSystemNode *p = node(index);

        if (p->hasInformation())
        {
            res = p->isDir();
        }
        else
        {
            res = fileInfo(index).isDir();
        }
    }

    return res;
}

qint64 QFileSystemShareModel::size(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;
    return node(index)->size();
}

QString QFileSystemShareModel::type(const QModelIndex &index) const
{
    if (!index.isValid()) return QString();
    return node(index)->type();
}

QDateTime QFileSystemShareModel::lastModified(const QModelIndex &index) const
{
    if (!index.isValid()) return QDateTime();
    return node(index)->lastModified();
}

QIcon QFileSystemShareModel::icon(const QModelIndex& index) const
{
    if (!index.isValid()) return QIcon();
    return node(index)->icon();
}

QString QFileSystemShareModel::displayName(const QModelIndex &index) const
{
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    shared_files_tree::QFileSystemNode *dirNode = node(index);
    if (!dirNode->volumeName.isNull())
        return dirNode->volumeName + QLatin1String(" (") + name(index) + QLatin1Char(')');
#endif
    return name(index);
}

QString QFileSystemShareModel::name(const QModelIndex &index) const
{
    if (!index.isValid()) return QString();
    /*shared_files_tree::QFileSystemNode *dirNode = node(index);

    if (dirNode->isSymLink() && fileInfoGatherer.resolveSymlinks())
    {
        QString fullPath = QDir::fromNativeSeparators(filePath(index));
        if (resolvedSymLinks.contains(fullPath))
            return resolvedSymLinks[fullPath];
    }
    */
    return node(index)->fileName;
}

QString QFileSystemShareModel::time(const QModelIndex& index) const
{
    if (!index.isValid()) return QString();
#ifndef QT_NO_DATESTRING
    return node(index)->lastModified().toString(Qt::SystemLocaleDate);
#else
    Q_UNUSED(index);
    return QString();
#endif
}

QFile::Permissions QFileSystemShareModel::permissions(const QModelIndex &index) const
{
    if (!index.isValid()) return QFile::Permissions();
    return node(index)->permissions();
}

QString QFileSystemShareModel::size(qint64 bytes) const
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

