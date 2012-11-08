#ifndef __TREE_H__
#define __TREE_H__

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "share_files.h"

class DirNode;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
     TreeModel(DirNode* root, QObject *parent = 0);
     ~TreeModel();

     QVariant data(const QModelIndex &index, int role) const;
     Qt::ItemFlags flags(const QModelIndex &index) const;
     QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;
     QModelIndex index(int row, int column,
                       const QModelIndex &parent = QModelIndex()) const;
     QModelIndex parent(const QModelIndex &index) const;
     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;
     bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
 private:

      DirNode*     m_rootItem;

};


#endif //__TREE_H__
