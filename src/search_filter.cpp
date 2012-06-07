#include "search_filter.h"

#include <QAction>
#include <QMenu>
#include <QStyle>
#include <QToolButton>
#include <QPushButton>

search_filter::search_filter(QWidget *parent)
    : QLineEdit(parent)
{
    btnFilter = new QToolButton(this);
    QIcon icon(":/emule/search/SearchFilter.png");
    btnFilter->setIcon(icon);
    btnFilter->setIconSize(QSize(16, 16));
    btnFilter->setMinimumSize(QSize(28, 16));
    btnFilter->setCursor(Qt::ArrowCursor);
    btnFilter->setPopupMode(QToolButton::InstantPopup);
    btnFilter->setStyleSheet("QToolButton { border: none; padding: 0px; }");

    clearButton = new QToolButton(this);
    QIcon icon2(":/emule/search/clear1.ico");
    clearButton->setIcon(icon2);
    clearButton->setIconSize(QSize(16, 16));
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    clearButton->hide();    

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-left: %2px; } ").arg(btnFilter->sizeHint().width() + frameWidth + 4));

    for (int ii = 0; ii < SWDelegate::SW_COLUMNS_NUM; ii++)
    {
        filterTypes[ii] = new QAction(this);
        filterTypes[ii]->setCheckable(true);
    }

    menuFilters = new QMenu(this);
    menuFilters->setObjectName(QString::fromUtf8("menuFilters"));

    filterTypes[SWDelegate::SW_NAME]->setText(tr("File Name"));
    filterTypes[SWDelegate::SW_SIZE]->setText(tr("File Size"));
    filterTypes[SWDelegate::SW_AVAILABILITY]->setText(tr("Availability"));
    filterTypes[SWDelegate::SW_SOURCES]->setText(tr("Sources"));
    filterTypes[SWDelegate::SW_TYPE]->setText(tr("Type"));
    filterTypes[SWDelegate::SW_ID]->setText(tr("ID"));
    filterTypes[SWDelegate::SW_DURATION]->setText(tr("Duration"));
    filterTypes[SWDelegate::SW_BITRATE]->setText(tr("Bitrate"));
    filterTypes[SWDelegate::SW_CODEC]->setText(tr("Codec"));

    for (int ii = 0; ii < SWDelegate::SW_COLUMNS_NUM; ii++)
    {
        menuFilters->addAction(filterTypes[ii]);
        connect(filterTypes[ii],  SIGNAL(triggered()), this, SLOT(setFilterType()));
    }

    btnFilter->setMenu(menuFilters);
    filterTypes[SWDelegate::SW_NAME]->setChecked(true);

    QPalette palette;
    palette.setColor(this->foregroundRole(), QColor(Qt::gray));
    this->setPalette(palette);

    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearFilter()));
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateFilter(const QString&)));

    emptyText = true;
    setText(filterTypes[SWDelegate::SW_NAME]->text());
    column = SWDelegate::SW_NAME;
}

search_filter::~search_filter()
{
    for (int ii = 0; ii < SWDelegate::SW_COLUMNS_NUM; ii++)
        delete filterTypes[ii];
}

void search_filter::resizeEvent(QResizeEvent *)
{
    QSize sz = btnFilter->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    btnFilter->move(rect().left() + frameWidth, (rect().bottom() + 2 - sz.height())/2);
    sz = clearButton->sizeHint();
    clearButton->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 2 - sz.height())/2);
}

void search_filter::setFilterType()
{    
    QAction* sender = qobject_cast<QAction*>(QObject::sender());

    for (int ii = 0; ii < SWDelegate::SW_COLUMNS_NUM; ii++)
    {
        if (sender == filterTypes[ii])
        {
            column = (SWDelegate::Column)ii;
            filterTypes[ii]->setChecked(true);
        }
        else
            filterTypes[ii]->setChecked(false);        
    }
    emit filterSelected(column);
    clearFocus();
}

void search_filter::updateFilter(const QString& text)
{
    if (!emptyText)
	    clearButton->setVisible(!text.isEmpty());
}

void search_filter::clearFilter()
{
    clear();
    clearFocus();
}

void search_filter::focusInEvent(QFocusEvent* event)
{    
    if (emptyText)
    {
        QPalette palette;
        palette.setColor(this->foregroundRole(), QColor(Qt::black));
        this->setPalette(palette);

        emptyText = false;
        setText("");
    }
    QLineEdit::focusInEvent(event);
}

void search_filter::focusOutEvent(QFocusEvent* event)
{
    if (!text().length())
    {
        emptyText = true;

        QPalette palette;
        palette.setColor(this->foregroundRole(), QColor(Qt::gray));
        this->setPalette(palette);

        setText(filterTypes[column]->text());
    }
    QLineEdit::focusOutEvent(event);
}