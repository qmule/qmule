//this file is part of xCatalog
//Copyright (C) 2011 xCatalog Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <QtGui/QApplication>
#include <QtGui/QPainter>

#include "fileitemdelegate.h"
#include "filemodel.h"
#include "config.h"


FileItemDelegate::FileItemDelegate()
{
}

void FileItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    const QWidget *widget = NULL;
    if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option))
        widget = v3->widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    // draw date at corner
    QDateTime itemDateTime = index.data(FileModel::DateRole).value<QDateTime>();
    const QDate &itemDate = itemDateTime.date();
    QDate currentDate = QDate::currentDate();
    QString dateString;

    if ( itemDate == currentDate )
        dateString = tr("Сегодня в %1").arg(itemDateTime.toString("hh:mm"));
    else if ( currentDate.addDays(-1) == itemDate )
        dateString = tr("Вчера в %1").arg(itemDateTime.toString("hh:mm"));
    else
        dateString = itemDateTime.toString("dd.MM.yyyy hh:mm");

    QTextOption to;
    to.setAlignment(Qt::AlignBottom|Qt::AlignRight);
    painter->setPen(QColor(Qt::darkGray));
    painter->drawText(option.rect, dateString, to);

    // draw catalog path for search
    QString folderPath = index.data(FileModel::PathRole).value<QString>();
    QFontMetrics fm = painter->fontMetrics();

    QRect pathRect = option.rect;
    pathRect.adjust(XCFG_THUMB_WIDTH+5,pathRect.height()-fm.height(),-85,0);
    to.setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    folderPath = fm.elidedText(folderPath, Qt::ElideRight,pathRect.width() );
    painter->setPen(QColor(Qt::darkGray));
    painter->drawText(pathRect, folderPath, to);

    // draw bottom gray line
    painter->setPen(QColor(Qt::lightGray));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
}
