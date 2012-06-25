#ifndef SEARCHWIDGETDELEGATE_H
#define SEARCHWIDGETDELEGATE_H

#include <QItemDelegate>
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
            case SW_SIZE:
            {
                drawDisplay(painter, option, option.rect, misc::friendlyUnit(index.data().toLongLong(), sizeType));
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
