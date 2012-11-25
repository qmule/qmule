#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


TreeNode::TreeNode(QList<QVariant> data, TreeNode* parent)
{
    m_data = data;
    m_parent = parent;
}

TreeNode::~TreeNode()
{
    qDeleteAll(m_childs);
}

int TreeNode::row() const
{
    int res = 0;
    if (m_parent) res = m_parent->m_childs.indexOf(const_cast<TreeNode*>(this));
    return res;
}

int TreeNode::columns() const
{
    return m_data.size();
}

int TreeNode::childs_count() const
{
    return (m_childs.size());
}

void TreeNode::appendNode(TreeNode* pnode)
{
    m_childs.append(pnode);
}

TreeNode* TreeNode::child(int row)
{
    return m_childs.value(row);
}

QVariant TreeNode::data(int col) const
{
    return m_data.value(col);
}

TreeNode* TreeNode::parent()
{
    return m_parent;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile file("./default.txt");
    file.open(QIODevice::ReadOnly);
    m_model = new TreeModel(file.readAll(), this);
    file.close();
    //m_model = new QFileSystemModel(this);
    //m_model->setRootPath(QString("C:\\"));
    ui->treeView->setModel(m_model);
}

TreeModel::TreeModel(const QString &data, QObject *parent /*= 0*/)
    :QAbstractItemModel(parent)
{
    QList<QVariant> header;
    header << "Title1" << "Title2";
    m_rootItem = new TreeNode(header, NULL);
    setupModelData(data.split("\n"), m_rootItem);
}

TreeModel::~TreeModel()
{
    delete m_rootItem;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    QVariant res;

    if (index.isValid() && role == Qt::DisplayRole)
    {
        res = static_cast<TreeNode*>(index.internalPointer())->data(index.column());
    }

    return res;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                    int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column,
                  const QModelIndex &parent/* = QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeNode* parent_item;

    if (!parent.isValid())
    {
        parent_item = m_rootItem;
    }
    else
    {
        parent_item = static_cast<TreeNode*>(parent.internalPointer());
    }

    TreeNode* child_item = parent_item->child(row);

    if (child_item)
    {
        return createIndex(row, column, child_item);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeNode* item = static_cast<TreeNode*>(index.internalPointer());

    if (item->parent() == m_rootItem)
    {
        return QModelIndex();
    }
    else
    {
        return createIndex(item->row(), 0, item->parent());
    }
}

int TreeModel::rowCount(const QModelIndex &parent/* = QModelIndex()*/) const
{
    TreeNode* parent_item;

    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parent_item = m_rootItem;
    else
        parent_item = (static_cast<TreeNode*>(parent.internalPointer()));

    parent_item->childs_count();
}

int TreeModel::columnCount(const QModelIndex &parent/* = QModelIndex()*/) const
{
    if (parent.isValid())
    {
        return (static_cast<TreeNode*>(parent.internalPointer()))->columns();
    }
    else
    {
        return m_rootItem->columns();
    }
}

void TreeModel::setupModelData(const QStringList &lines, TreeNode *parent)
{
    QList<TreeNode*> parents;
         QList<int> indentations;
         parents << parent;
         indentations << 0;

         int number = 0;

         while (number < lines.count()) {
             int position = 0;
             while (position < lines[number].length()) {
                 if (lines[number].mid(position, 1) != " ")
                     break;
                 position++;
             }

             QString lineData = lines[number].mid(position).trimmed();

             if (!lineData.isEmpty()) {
                 // Read the column data from the rest of the line.
                 QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
                 QList<QVariant> columnData;
                 for (int column = 0; column < columnStrings.count(); ++column)
                     columnData << columnStrings[column];

                 if (position > indentations.last()) {
                     // The last child of the current parent is now the new parent
                     // unless the current parent has no children.

                     if (parents.last()->childs_count() > 0) {
                         parents << parents.last()->child(parents.last()->childs_count()-1);
                         indentations << position;
                     }
                 } else {
                     while (position < indentations.last() && parents.count() > 0) {
                         parents.pop_back();
                         indentations.pop_back();
                     }
                 }

                 // Append a new item to the current parent's list of children.
                 qDebug() << "add data " << columnData;
                 parents.last()->appendNode(new TreeNode(columnData, parents.last()));
             }

             number++;
         }
}

MainWindow::~MainWindow()
{
    delete ui;
}
