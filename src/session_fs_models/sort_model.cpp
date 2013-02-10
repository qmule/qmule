#include "sort_model.h"
#include "base_model.h"
#include "path_model.h"


QChar getNextChar(const QString &s, int location)
{
    return (location < s.length()) ? s.at(location) : QChar();
}

int naturalCompare(const QString &s1, const QString &s2,  Qt::CaseSensitivity cs)
{
    for (int l1 = 0, l2 = 0; l1 <= s1.count() && l2 <= s2.count(); ++l1, ++l2) {
        // skip spaces, tabs and 0's
        QChar c1 = getNextChar(s1, l1);
        while (c1.isSpace())
            c1 = getNextChar(s1, ++l1);
        QChar c2 = getNextChar(s2, l2);
        while (c2.isSpace())
            c2 = getNextChar(s2, ++l2);

        if (c1.isDigit() && c2.isDigit()) {
            while (c1.digitValue() == 0)
                c1 = getNextChar(s1, ++l1);
            while (c2.digitValue() == 0)
                c2 = getNextChar(s2, ++l2);

            int lookAheadLocation1 = l1;
            int lookAheadLocation2 = l2;
            int currentReturnValue = 0;
            // find the last digit, setting currentReturnValue as we go if it isn't equal
            for (
                QChar lookAhead1 = c1, lookAhead2 = c2;
                (lookAheadLocation1 <= s1.length() && lookAheadLocation2 <= s2.length());
                lookAhead1 = getNextChar(s1, ++lookAheadLocation1),
                lookAhead2 = getNextChar(s2, ++lookAheadLocation2)
                ) {
                bool is1ADigit = !lookAhead1.isNull() && lookAhead1.isDigit();
                bool is2ADigit = !lookAhead2.isNull() && lookAhead2.isDigit();
                if (!is1ADigit && !is2ADigit)
                    break;
                if (!is1ADigit)
                    return -1;
                if (!is2ADigit)
                    return 1;
                if (currentReturnValue == 0) {
                    if (lookAhead1 < lookAhead2) {
                        currentReturnValue = -1;
                    } else if (lookAhead1 > lookAhead2) {
                        currentReturnValue = 1;
                    }
                }
            }
            if (currentReturnValue != 0)
                return currentReturnValue;
        }

        if (cs == Qt::CaseInsensitive) {
            if (!c1.isLower()) c1 = c1.toLower();
            if (!c2.isLower()) c2 = c2.toLower();
        }
        int r = QString::localeAwareCompare(c1, c2);
        if (r < 0)
            return -1;
        if (r > 0)
            return 1;
    }
    // The two strings are the same (02 == 2) so fall back to the normal sort
    return QString::compare(s1, s2, cs);
}


SessionFilesSort::SessionFilesSort(QObject* parent /* = 0*/) : QSortFilterProxyModel(parent)
{
    setSortRole(BaseModel::SortRole);
}

bool SessionFilesSort::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if (left.column() == BaseModel::DC_NAME)
    {
        return (naturalCompare(sourceModel()->data(left, Qt::DisplayRole).toString(),
                               sourceModel()->data(right, Qt::DisplayRole).toString(),
                               Qt::CaseSensitive) < 0);
    }

    return (QSortFilterProxyModel::lessThan(left, right));
}

bool SessionDirectoriesSort::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if (left.column() == BaseModel::DC_NAME)
    {
        return (naturalCompare(sourceModel()->data(left, Qt::DisplayRole).toString(),
                               sourceModel()->data(right, Qt::DisplayRole).toString(),
                               Qt::CaseSensitive) < 0);
    }

    return (QSortFilterProxyModel::lessThan(left, right));
}

bool PathsSort::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if (left.column() == 0)
    {
        if (left.row() < PathModel::filters_count)
        {
            return (sortOrder() == Qt::AscendingOrder);
        }
        else if (right.row() < PathModel::filters_count)
        {
            return (sortOrder() != Qt::AscendingOrder);
        }
        else
        {
            return (naturalCompare(sourceModel()->data(left, Qt::DisplayRole).toString(),
                               sourceModel()->data(right, Qt::DisplayRole).toString(),
                               Qt::CaseSensitive) < 0);
        }
    }

    return (QSortFilterProxyModel::lessThan(left, right));
}
