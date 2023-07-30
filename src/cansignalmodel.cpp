#include "cansignalmodel.h"

QVariant CanSignalModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    if (role == Qt::DisplayRole) {
        auto signal = db.at(msgIndex).canSignals.at(index.row());
        QString ret = "";
        switch (index.column()) {
        case signalName:
            ret = signal.name;
            break;
        case signalStartBit:
            ret = QString::number(signal.startBit);
            break;
        case signalLen:
            ret = QString::number(signal.len);
            break;
        case signalScale:
            ret = QString("%1").arg(signal.scale);
            break;
        case signalMax:
            ret = QString("%1").arg(signal.max);
            break;
        case signalMin:
            ret = QString("%1").arg(signal.min);
            break;
        case signalOffset:
            ret = QString("%1").arg(signal.offset);
            break;
        case signalEndian:
            if (signal.isBigEndian) {
                ret = "big";
            } else {
                ret = "little";
            }
            break;
        case signalType:
            if (signal.isSigned) {
                ret = "signed";
            } else {
                ret = "unsigned";
            }
            break;
        case signalUnit:
            ret = signal.unit;
            break;
        case signalValues:
            auto pairs = signal.getValuePairs();
            ret = "";
            for (auto p : pairs) {
                ret += QString("%1 - %2\n").arg(p.first).arg(p.second);
            }
            ret = ret.trimmed();
            break;
        }
        return ret;
    } else
        return {};
}

QVariant CanSignalModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    static QVector<QString> const headers = {
        tr("Name"),   tr("Start Bit"), tr("Len"),    tr("Type"),
        tr("Endian"), tr("Scale"),     tr("Offset"), tr("Min"),
        tr("Max"),    tr("Unit"),      tr("Values")
    };
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal) {
        if (section >= signalMaxCol)
            return {};
        else
            return headers.at(section);
    } else
        return {};
}
