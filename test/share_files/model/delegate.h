#ifndef __DELEGATE__
#define __DELEGATE__

#include <QItemDelegate>
#include <QDebug>

class CheckBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    CheckBoxDelegate(QObject *parent = 0);
/*
    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        //qDebug() << "paint ";
        QAbstractItemView* view = dynamic_cast<QAbstractItemView*>(parent());
        if (!view->indexWidget(index))
        {
            view->setIndexWidget(index, this);
        }
        else
        {
            QItemDelegate::paint(painter, option, index);
        }
    }
    */

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
 };


#endif
