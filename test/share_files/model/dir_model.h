#ifndef __DIR_MODEL__H__
#define __DIR_MODEL__H__

#include "base_model.h"

class DirectoryModel : public BaseModel
{
public:
    DirectoryModel(DirNode* root, QObject* parent = 0);
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
    QModelIndex index(const FileNode* node);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
protected:
    virtual QModelIndex node2index(const FileNode*) const;
    virtual int node2row(const FileNode*) const;
    virtual int colcount() const { return 1; }
};

#endif // __DIR_MODEL__H__
