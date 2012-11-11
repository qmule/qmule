#include "delegate.h"
#include "share_files.h"
#include <QCheckBox>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent/* = 0*/) : QItemDelegate(parent)
{
}

QWidget* CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QCheckBox* editor = new QCheckBox(parent);
    return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid())
    {
        QCheckBox* cb = static_cast<QCheckBox*>(editor);
        cb->setChecked(index.model()->data(index, Qt::DisplayRole).toBool());
    }
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    Q_UNUSED(model);

    if (index.isValid())
    {
        QCheckBox* cb = static_cast<QCheckBox*>(editor);
        FileNode* node = static_cast<FileNode*>(index.internalPointer());
        Q_ASSERT(node);

        if (cb->isChecked())
        {
            node->share(false);
        }
        else
        {
            node->unshare(false);
        }
    }
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
