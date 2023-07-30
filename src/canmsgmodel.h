#pragma once
#include <memory>
#include <QAbstractItemModel>
#include "canmsg.h"

class CanMsgModel : public QAbstractListModel
{
    Q_OBJECT
public:
    CanMsgModel(CanDb& db, QObject *parent);

    static constexpr int columnCnt = 5;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount([[maybe_unused]] const QModelIndex &parent =
                            QModelIndex()) const override
    {
        return columnCnt;
    };

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int, Qt::Orientation, int) const override;

    QVariant getMsgId(QModelIndex index) const;

    const QList<CanSignal> &getSignals(const QModelIndex &index);

public slots:
    void dbChanged() {
        beginResetModel();
        endResetModel();
    }

private:
    CanDb& db;
};
