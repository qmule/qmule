#ifndef SEARCHWIDGETDELEGATE_H
#define SEARCHWIDGETDELEGATE_H

#include <QItemDelegate>
#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QLabel>

#include "misc.h"

extern QColor itemColor(const QModelIndex& inx);

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

    void paint(QPainter * painter, const QStyleOptionViewItem & opt, const QModelIndex & index) const 
    {
        QStyleOptionViewItem option(opt);
        option.palette.setColor(QPalette::Text, itemColor(index));

        switch (index.column())
        {
            case SW_NAME:
            {
                QString name = index.data().toString();
                if (misc::isTorrentLink(name))
                {
                    // this is a link to torrent tracker
                    QAbstractItemView* view = dynamic_cast<QAbstractItemView*>(parent());
                    if (!view->indexWidget(index))
                    {
                        // link isn't set yet
                        QString iconLink = "<img src=\":/emule/common/hyperlink.ico\">";
                        QLabel* label = new QLabel(
                            "<tr><td>" + iconLink + "</td><td>&nbsp;" + name + "</td></tr>", view);
                        label->setOpenExternalLinks(true);
                        label->setToolTip(name);
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
