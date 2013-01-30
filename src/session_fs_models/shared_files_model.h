#ifndef __SHARED_FILES_MODEL__
#define __SHARED_FILES_MODEL__

#include <QScopedPointer>
#include "file_model.h"
#include "file_filter.h"
#include "transport/session.h"

class SFModel : public FilesModel
{
    Q_OBJECT
public:
    SFModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex()) const;
    void setFilter(BaseFilter*);
public slots:
    void on_removeSharedFile(FileNode*);
    void on_insertSharedFile(FileNode*);
private:    
    QList<FileNode*>    m_files;
    QScopedPointer<BaseFilter> m_filter;
    int node2row(const FileNode*) const;
    void sync();
};

#endif //__SHARED_FILES_MODEL__
