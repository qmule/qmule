#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>

namespace Ui {
class MainWindow;
}

class TreeNode
{
public:
    TreeNode(QList<QVariant> data, TreeNode* parent);
    ~TreeNode();
    int row() const;
    int columns() const;
    int childs_count() const;
    void appendNode(TreeNode* pnode);
    TreeNode* child(int row);
    QVariant data(int col) const;
    TreeNode* parent();

private:
    TreeNode* m_parent;
    QList<TreeNode*> m_childs;
    QList<QVariant> m_data;
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QString &data, QObject *parent = 0);
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

private:
    void setupModelData(const QStringList &lines, TreeNode *parent);

    TreeNode *m_rootItem;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    TreeModel* m_model;
};

#endif // MAINWINDOW_H
