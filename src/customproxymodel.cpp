#include <QFont>
#include "customproxymodel.h"

void CustomProxyModel::setFilter(uint8_t column, const QString &expression)
{
    if (expression.isEmpty()) {
        regex.remove(column);
    } else {
        regex[column] = QRegularExpression(
                expression, QRegularExpression::CaseInsensitiveOption);
    }
    invalidateFilter();
}

bool CustomProxyModel::filterAcceptsRow(int sourceRow,
                                        const QModelIndex &parent) const
{
    auto match = true;
    if (parent.isValid()) {
        return true;
    }
    for (auto it = regex.begin(); it != regex.end(); it++) {
        auto index = sourceModel()->index(sourceRow, it.key());
        auto contain =
                sourceModel()->data(index).toString().contains(it.value());
        if (!contain)
            return false;
    }
    return true;
}

QVariant CustomProxyModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if ((role != Qt::FontRole) || (orientation != Qt::Horizontal)
        || (!regex.contains(section)))
        return sourceModel()->headerData(section, orientation, role);

    QFont font;
    font.setBold(true);
    return font;
}
