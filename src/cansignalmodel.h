#pragma once
#include <QAbstractItemModel>
#include "canmsg.h"

class CanSignalModel : public QAbstractListModel
{
    Q_OBJECT

public:
    using signalCol = enum {
        signalName,
        signalStartBit,
        signalLen,
        signalType,
        signalEndian,
        signalScale,
        signalOffset,
        signalMin,
        signalMax,
        signalUnit,
        signalValues,
        signalMaxCol
    };

    CanSignalModel(CanDb& db, QObject *parent = nullptr) : QAbstractListModel(parent), db(db) {};
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    int rowCount([[maybe_unused]] const QModelIndex &parent =
                         QModelIndex()) const override
    {
        if (msgIndex < 0)
            return 0;
        return static_cast<int>(db.at(msgIndex).signalCount());
    }

    int columnCount([[maybe_unused]] const QModelIndex &parent =
                            QModelIndex()) const override
    {
        return signalMaxCol;
    }

    void setMsgIndex(int32_t idx)
    {
        beginResetModel();
        msgIndex = idx;
        endResetModel();
    };

public slots:
    void dbChanged() {
        beginResetModel();
        msgIndex = -1;
        endResetModel();
    }

private:
    int32_t msgIndex{-1};
    CanDb& db;
};
