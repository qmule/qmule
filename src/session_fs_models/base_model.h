#ifndef __BASE_MODEL__
#define __BASE_MODEL__

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "transport/session_filesystem.h"

class BaseModel : public QAbstractItemModel
{
    Q_OBJECT
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
         DC_STATUS = 0,
         DC_NAME,
         DC_SIZE,
         DC_TYPE,
         DC_TIME,
         DC_HASH
     };

     BaseModel(DirNode* root, QObject *parent = 0);
     virtual ~BaseModel();

     QModelIndex parent(const QModelIndex &index) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;

     QModelIndex index(const FileNode* node);
     QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex()) const;

     void setRootNode(const QModelIndex& index);

     // interface function
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
     virtual QModelIndex node2index(const FileNode*) const = 0;
     virtual int node2row(const FileNode*) const = 0;
     virtual int colcount() const = 0;

     DirNode*   m_rootItem;
     bool       m_row_count_changed;
     QFileIconProvider  m_iconProvider;
public slots:

    void changeNode(const FileNode* node);
    void beginRemoveNode(const FileNode* node);
    void endRemoveNode();
    void beginInsertNode(const FileNode* node, int pos);
    void endInsertNode();
};


#endif

