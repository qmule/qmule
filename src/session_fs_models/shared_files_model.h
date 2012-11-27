#ifndef __SHARED_FILES_MODEL__
#define __SHARED_FILES_MODEL__

#include "file_model.h"
#include "transport/session.h"

class SFModel : public FilesModel
{
    Q_OBJECT
public:
    SFModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex()) const;
    void setFilter(const QString&);
public slots:
    void on_removeSharedFile(FileNode*);
    void on_insertSharedFile(FileNode*);
private:
    QString             m_filter;
    QList<FileNode*>    m_files;
    int node2row(const FileNode*) const;
    void sync();
};

#endif //__SHARED_FILES_MODEL__
