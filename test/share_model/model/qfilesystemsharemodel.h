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
    QFileSystemShareModel(shared_files_tree* pSFTree);
    ~QFileSystemShareModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole) const;
     QModelIndex index(int row, int column,
                       const QModelIndex &parent = QModelIndex()) const;
     QModelIndex parent(const QModelIndex &index) const;
     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;
private:

    shared_files_tree* m_pSFTree;
    QBasicTimer fetchingTimer;
    struct Fetching
    {
        QString dir;
        QString file;
        const shared_files_tree::QFileSystemNode *node;
    };
};

#endif // QFILESYSTEMSHAREMODEL_H
