#ifndef __FILE_MODEL__H__
#define __FILE_MODEL__H__

#include "base_model.h"

class FilesModel : public BaseModel
{
public:
    FilesModel(DirNode* root, QObject* parent = 0);
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
    QModelIndex index(const FileNode* node);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
protected:
    virtual QModelIndex node2index(const FileNode*) const;
    virtual int node2row(const FileNode*) const;    
    virtual int colcount() const { return 7; };
};


#endif // __FILE_MODEL__H__
