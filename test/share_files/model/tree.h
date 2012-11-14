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
     enum Filter { File = 0x01, Dir = 0x02, All = File | Dir };
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
         DC_STATUS = 0,
         DC_NAME,
         DC_SIZE,
         DC_TYPE,
         DC_TIME,
         DC_HASH
     };

     TreeModel(DirNode* root, Filter filter, QObject *parent = 0);
     ~TreeModel();

     bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
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

     void setRootNode(const QModelIndex& index);
     QModelIndex index(const FileNode* node);
     QString hash(const QModelIndex& index) const;
     bool contains_active_children(const QModelIndex& index) const;
     bool active(const QModelIndex& index) const;
     qint64 size(const QModelIndex &index) const;
     QString type(const QModelIndex &index) const;
     QDateTime lastModified(const QModelIndex &index) const;
     QIcon icon(const QModelIndex& index) const;
     QString displayName(const QModelIndex &index) const;
     QString name(const QModelIndex& index) const;
     QString filepath(const QModelIndex& index) const;
     QString time(const QModelIndex& index) const;
     QFile::Permissions permissions(const QModelIndex &index) const;
     QString size(qint64 bytes) const;
protected:
     int elements_count(const DirNode* node) const;
     QVariant displayName(const QModelIndex& index);
     FileNode* node(const QModelIndex& index) const;
     DirNode*   m_rootItem;
     Filter     m_filter;
     QFileIconProvider  m_iconProvider;
     bool m_change;
public slots:
    void removeNode(const FileNode* node);
    void addNode(const FileNode* node);
    void changeNode(const FileNode* node);

    void beginRemoveNode(const FileNode* node);
    void endRemoveNode();
    void beginInsertNode(const FileNode* node, int pos);
    void endInsertNode();
};

class DirModel : public TreeModel
{
public:
    DirModel(DirNode* root, QObject* parent = 0) : TreeModel(root, TreeModel::Dir, parent){}
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const { return 1; }
};


#endif //__TREE_H__
