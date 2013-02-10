#include "path_model.h"
#include "session.h"

PathModel::PathModel(QObject *parent /* = 0*/)
    : QAbstractListModel(parent)
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
    return m_paths.size() + filters_count;
}

QVariant PathModel::data(const QModelIndex &index, int role) const
{
    QVariant res;

    if (!index.isValid())
        return res;

    if (index.row() > m_paths.size() + filters_count)
        return res;

    if (role == Qt::DisplayRole)
    {
        switch(index.row())
        {
            case libed2k::ED2KFT_ANY:
                res = tr("Any");
                break;
            case libed2k::ED2KFT_AUDIO:
                res = tr("Audios");
                break;
            case libed2k::ED2KFT_VIDEO:
                res = tr("Videos");
                break;
            case libed2k::ED2KFT_IMAGE:
                res = tr("Pictures");
                break;
            case libed2k::ED2KFT_PROGRAM:
                res = tr("Programs");
                break;
            case libed2k::ED2KFT_DOCUMENT:
                res = tr("Documents");
                break;
            case libed2k::ED2KFT_ARCHIVE:
                res = tr("Archives");
                break;
            case libed2k::ED2KFT_CDIMAGE:
                res = tr("CD images");
                break;
            case libed2k::ED2KFT_EMULECOLLECTION:
                res = tr("Emule collections");
                break;
            case all_files:
                res = tr("All shared files");
                break;
            default:
                res = m_paths.at(index.row() - filters_count)->filepath();
                break;
        }
    }
    else if (role == Qt::DecorationRole)
    {
        switch(index.row())
        {
            case libed2k::ED2KFT_ANY:
                res = QIcon(":/emule/common/FileTypeAny.ico");
                break;
            case libed2k::ED2KFT_AUDIO:
                res = QIcon(":/emule/common/FileTypeAudio.ico");
                break;
            case libed2k::ED2KFT_VIDEO:
                res = QIcon(":/emule/common/FileTypeVideo.ico");
                break;
            case libed2k::ED2KFT_IMAGE:
                res = QIcon(":/emule/common/FileTypePicture.ico");
                break;
            case libed2k::ED2KFT_PROGRAM:
                res = QIcon(":/emule/common/FileTypeProgram.ico");
                break;
            case libed2k::ED2KFT_DOCUMENT:
                res = QIcon(":/emule/common/FileTypeDocument.ico");
                break;
            case libed2k::ED2KFT_ARCHIVE:
                res = QIcon(":/emule/common/FileTypeArchive.ico");
                break;
            case libed2k::ED2KFT_CDIMAGE:
                res = QIcon(":/emule/common/FileTypeCDImage.ico");
                break;
            case libed2k::ED2KFT_EMULECOLLECTION:
                res = QIcon(":/emule/common/FileTypeEmuleCollection.ico");
                break;
            case all_files:
                res = QIcon(":/emule/files/all.ico");
                break;
            default:
                res = QIcon(":/emule/common/folder_share.ico");
                break;
        }
    }

    return res;
}

QVariant PathModel::headerData(int section, Qt::Orientation orientation,
                    int role /*= Qt::DisplayRole*/) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return tr("Filter/Filepath");

    return QVariant();
}

const DirNode* PathModel::node(const QModelIndex& indx) const
{
    const DirNode* res = Session::instance()->root();

    if (indx.isValid())
    {
        if (indx.row() >= filters_count)
        {
            res = m_paths.at(indx.row() - filters_count);
        }
    }

    return res;
}

const QString PathModel::filepath(const QModelIndex& indx) const
{
    QString res;

    if (indx.isValid())
    {
        if (indx.row() >= filters_count)
        {
            res = m_paths.at(indx.row() - filters_count)->filepath();
        }
    }

    return res;
}

BaseFilter* PathModel::filter(const QModelIndex& index) const
{
    BaseFilter* res = NULL;

    if (index.isValid())
    {
        if (index.row() == all_files)
        {
            res = new BaseFilter();
        }
        else if (index.row() < filters_count)
        {
            res = new TypeFilter((libed2k::EED2KFileType)index.row());
        }
        else
        {
            res = new PathFilter(m_paths.at(index.row() - filters_count)->filepath());
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
