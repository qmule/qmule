#ifndef __SORT_MODEL__
#define __SORT_MODEL__

#include <QSortFilterProxyModel>


class SessionFilesSort : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SessionFilesSort(QObject* parent = 0);
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};

class SessionDirectoriesSort : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SessionDirectoriesSort(QObject* parent = 0) : QSortFilterProxyModel(parent) {}
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};

#endif //__SORT_MODEL__
