#ifndef SEARCHWIDGETDELEGATE_H
#define SEARCHWIDGETDELEGATE_H

#include <QItemDelegate>
#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QLabel>

#include "misc.h"

class SWDelegate: public QItemDelegate {
  Q_OBJECT

public:

    enum Column 
    {
        SW_NAME, 
        SW_SIZE, 
        SW_AVAILABILITY, 
        SW_SOURCES, 
        SW_TYPE, 
        SW_ID,
        SW_DURATION,
        SW_BITRATE,
        SW_CODEC,
        SW_COLUMNS_NUM
    };

public:
    SWDelegate(QObject *parent) : QItemDelegate(parent) 
    {
        sizeType = misc::ST_DEFAULT;
    }

    ~SWDelegate() {}

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const 
    {
        switch (index.column())
        {
            case SW_NAME:
            {
                QAbstractItemView* view = dynamic_cast<QAbstractItemView*>(parent());
                QString name = index.data().toString();
                if (name.startsWith("<a"))
                {
                    // this is a link to torrent tracker
                    if (!view->indexWidget(index))
                    {
                        QLabel* label = new QLabel(name, view);
                        label->setOpenExternalLinks(true);
                        label->setToolTip(label->text());
                        view->setIndexWidget(index, label);
                    }
                    drawDisplay(painter, option, option.rect, "");
                }
                else
                {
                    QItemDelegate::paint(painter, option, index);
                }
                break;
            }
            case SW_SIZE:
            {
                drawDisplay(painter, option, option.rect,
                            misc::friendlyUnit(index.data().toLongLong(), sizeType));
                break;
            }
            case SW_SOURCES:
            {
                const QAbstractItemModel* model = index.model();
                int sources = model->data(
                    model->index(index.row(), SWDelegate::SW_AVAILABILITY)).toInt();
                int completeSources = index.data().toInt();

                QString strSrc = (sources > 0) ?
                    (QString::number(100 * completeSources / sources)) : "0";
                strSrc += "%(";
                strSrc += QString::number(completeSources);
                strSrc += ")";
                drawDisplay(painter, option, option.rect, strSrc);
                break;
            }
            case SW_DURATION:
            {
                qlonglong seconds = index.data().toLongLong();
                if (seconds > 0)
                    drawDisplay(painter, option, option.rect, misc::userFriendlyDuration(seconds, 1));
                else
                    drawDisplay(painter, option, option.rect, "");
                break;
            }
            case SW_BITRATE:
            {
                QString bitRate = index.data().toString();
                if (bitRate.length() && index.data().toInt() > 0) 
                {
                    bitRate += tr(" kBit/s");
                    drawDisplay(painter, option, option.rect, bitRate);
                }
                else
                    drawDisplay(painter, option, option.rect, "");
                break;
            }

            default:
                QItemDelegate::paint(painter, option, index);
        }
    }

    void setSizeType(misc::SizeType type)
    {
        sizeType = type;
    }

private:
    misc::SizeType sizeType;
};

#endif
