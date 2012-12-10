#ifndef QFILESYSTEMSHAREMODEL_H
#define QFILESYSTEMSHAREMODEL_H

#include <QAbstractItemModel>
#include <QString>
#include <QList>
#include <QHash>
#include <QDateTime>
#include <QFile>
#include <QIcon>
#include <QBasicTimer>

#include "shared_files_tree.h"
#include "qfileinfogatherer.h"

class QFileSystemShareModel : public QAbstractItemModel
{
public:

    // TODO - add transfer related items
    enum Roles
    {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole = Qt::UserRole + 2,
        FilePermissions = Qt::UserRole + 3
    };

    enum DisplayColumns
    {
        DC_NAME = 0,
        DC_SIZE,
        DC_TYPE,
        DC_TIME
    };

    QFileSystemShareModel(shared_files_tree* pSFTree);
    ~QFileSystemShareModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(const QString& path, int column = 0) const;
    QModelIndex parent(const QModelIndex &index) const;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    // special members
    inline QFileInfo fileInfo(const QModelIndex &index) const;
    QString filePath(const QModelIndex &index) const;
    bool isDir(const QModelIndex &index) const;
    qint64 size(const QModelIndex &index) const;
    QString type(const QModelIndex &index) const;
    QDateTime lastModified(const QModelIndex &index) const;
    QIcon icon(const QModelIndex& index) const;
    QString displayName(const QModelIndex &index) const;
    QString name(const QModelIndex& index) const;
    QString time(const QModelIndex& index) const;
    QFile::Permissions permissions(const QModelIndex &index) const;
    QString size(qint64 bytes) const;
private:

    shared_files_tree::QFileSystemNode* node(const QModelIndex& index) const;
    QModelIndex index(const shared_files_tree::QFileSystemNode* node) const;

    shared_files_tree* m_pSFTree;
    QBasicTimer fetchingTimer;
    struct Fetching
    {
        QString dir;
        QString file;
        const shared_files_tree::QFileSystemNode *node;
    };
};

inline QFileInfo QFileSystemShareModel::fileInfo(const QModelIndex &aindex) const
{ return QFileInfo(filePath(aindex)); }

#endif // QFILESYSTEMSHAREMODEL_H
