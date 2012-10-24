#include "qfilesystemsharemodel.h"

QFileSystemShareModel::QFileSystemShareModel(shared_files_tree* pSFTree) : m_pSFTree(pSFTree)
{
}

QFileSystemShareModel::~QFileSystemShareModel()
{
}

QVariant QFileSystemShareModel::data(const QModelIndex &index, int role) const
{

}

Qt::ItemFlags QFileSystemShareModel::flags(const QModelIndex &index) const
{

}

QVariant QFileSystemShareModel::headerData(int section, Qt::Orientation orientation,
                         int role /*Qt::DisplayRole*/) const
{
}

 QModelIndex QFileSystemShareModel::index(int row, int column,
                   const QModelIndex &parent /* QModelIndex()*/) const
 {
     if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
         return QModelIndex();

     shared_files_tree::QFileSystemNode* parent_node;

     if (!parent.isValid())
     {
        parent_node = m_pSFTree->rootNode();
     }
     else
     {
        parent_node = static_cast<shared_files_tree::QFileSystemNode*>(parent.internalPointer());
     }

     Q_ASSERT(parent_node);

     //shared_files_tree::QFileSystemNode* node = parent_node->node(row);
     //Q_ASSERT(node);
     //return createIndex(row, column, node);
 }

 QModelIndex QFileSystemShareModel::parent(const QModelIndex &index) const
 {
     if (!index.isValid())  return QModelIndex();
     shared_files_tree::QFileSystemNode* node = static_cast<shared_files_tree::QFileSystemNode*>(index.internalPointer());
     shared_files_tree::QFileSystemNode* parent_node = node->parent;

     if (!parent_node || parent_node == m_pSFTree->rootNode())
     {
         return QModelIndex();
     }

     shared_files_tree::QFileSystemNode* grand_parent_node = parent_node->parent;
     Q_ASSERT(grand_parent_node->children.contains(parent_node->fileName));

     //return createIndex(grand_parent_node->visibleChildren.);
 }

 int QFileSystemShareModel::rowCount(const QModelIndex &parent /* QModelIndex()*/) const
 {
     if (parent.column() > 0)
         return 0;

     shared_files_tree::QFileSystemNode* parent_node;

     if (!parent.isValid())
     {
         parent_node = m_pSFTree->rootNode();
     }
     else
     {
         parent_node = static_cast<shared_files_tree::QFileSystemNode*>(parent.internalPointer());
     }

     Q_ASSERT(parent_node);

     return parent_node->children.count();
 }

 int QFileSystemShareModel::columnCount(const QModelIndex &parent /* = QModelIndex()*/) const
 {
     return (parent.column() > 0) ? 0 : 4;
 }
