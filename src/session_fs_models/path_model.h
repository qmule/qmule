#ifndef __PATH_MODEL__
#define __PATH_MODEL__

#include <QAbstractListModel>
#include <QIcon>
#include "transport/session.h"
#include "file_filter.h"

class PathModel : public QAbstractListModel
{
    Q_OBJECT
public:
    // additional all files row
    enum { filters_count = libed2k::ED2KFT_EMULECOLLECTION + 2 };
    enum { all_files = libed2k::ED2KFT_EMULECOLLECTION + 1 };
    PathModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    const DirNode* node(const QModelIndex&) const;
    const QString filepath(const QModelIndex&) const;
    BaseFilter* filter(const QModelIndex&) const;
public slots:
    void on_removeSharedDirectory(const DirNode*);
    void on_insertSharedDirectory(const DirNode*);
private:
    QList<const DirNode*> m_paths;
    int node2row(const DirNode*) const;
};

#endif // __PATH_MODEL__
