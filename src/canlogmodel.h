#pragma once

#include <QSortFilterProxyModel>
#include <QVector>
#include <QHash>

#include "canmsg.h"

class CanLogModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CanLogModel(const QVector<CanLogMsg> &buffer, const CanDb &db, QObject *parent = nullptr);

    int rowCount([[maybe_unused]] const QModelIndex &parent =
                         QModelIndex()) const override;
    int columnCount([[maybe_unused]] const QModelIndex &parent =
                            QModelIndex()) const override
    {
        return 7;
    }
    QModelIndex index(int row, int column,
                      const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool isMsgHighlight(const QModelIndex &index);
    bool isIdHighlight(const QModelIndex &index);
    void setHighlightMsg(const QModelIndex &index, bool status);
    void setHighlightId(const QModelIndex &index, bool status);

 public slots:
    void logChanged() {
        beginResetModel();
        rowList.clear();
        idList.clear();
        endResetModel();
    }

    void dbChanged() {
        beginResetModel();
        endResetModel();
    }

private:
    QVariant displayRowData(const QModelIndex &index) const;
    QHash<uint32_t, bool> rowList;
    QHash<uint32_t, bool> idList;
    const QVector<CanLogMsg>& buffer;
    const CanDb& db;
};
