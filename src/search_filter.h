#ifndef SEARCH_FILTER_H
#define SEARCH_FILTER_H

#include <QLineEdit>

#include "search_widget_delegate.h"

QT_BEGIN_NAMESPACE
class QToolButton;
QT_END_NAMESPACE

class search_filter : public QLineEdit
{
    Q_OBJECT

public:
    search_filter(QWidget *parent);
    ~search_filter();
    bool isFilterSet() { return !emptyText; }

private:
    QToolButton *btnFilter;
    QToolButton *clearButton;
    QMenu* menuFilters;
    QAction* filterTypes[SWDelegate::SW_COLUMNS_NUM];
    SWDelegate::Column column;
    bool emptyText;

protected:
	void resizeEvent(QResizeEvent *);

    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);

private slots:
    void setFilterType();
    void clearFilter();
    void updateFilter(const QString &text);

signals:
    void filterSelected(SWDelegate::Column column);
};

#endif // SEARCH_FILTER_H
