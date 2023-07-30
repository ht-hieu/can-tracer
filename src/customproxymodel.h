#pragma once
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QHash>

class CustomProxyModel : public QSortFilterProxyModel
{
public:
    CustomProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent)
    {
    }

    QString getFilterText(uint8_t column) const
    {
        if (!regex.contains(column)) {
            return "";
        }
        auto re = regex.value(column);
        return re.pattern();
    }

    void setFilter(uint8_t column, const QString &expression);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    QHash<uint8_t, QRegularExpression> regex;
};
