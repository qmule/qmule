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
    QFileSystemShareModel();

private:

    QBasicTimer fetchingTimer;
    struct Fetching
    {
        QString dir;
        QString file;
        const shared_files_tree::QFileSystemNode *node;
    };
};

#endif // QFILESYSTEMSHAREMODEL_H
