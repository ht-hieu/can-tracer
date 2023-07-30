#include "canlogmodel.h"
#include <QString>
#include <QColor>
#include <QBrush>

CanLogModel::CanLogModel(const QVector<CanLogMsg> &buffer, const CanDb &db,
                         QObject *parent)
    : QAbstractItemModel(parent),
      rowList(QHash<uint32_t, bool>()),
      idList(QHash<uint32_t, bool>()),
      buffer(buffer),
      db(db)
{
}

int CanLogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.constInternalPointer() != nullptr) {
            return 0;
        }
        auto log = buffer.at(parent.row());
        auto msg = db.findMessage(log.id);
        if (msg == nullptr) {
            return 0;
        }
        return msg->signalCount();
    } else {
        return buffer.size();
    }
}

QVariant CanLogModel::displayRowData(const QModelIndex &index) const
{
    QString ret = "";
    const CanMessage *msg = nullptr;
    if (index.constInternalPointer() == nullptr) {
        auto item = buffer.at(index.row());
        msg = db.findMessage(item.id);

        switch (index.column()) {
        case 0:
            ret = QString::number(item.number);
            break;
        case 1:
            ret = QString::number(item.time, 'f', 4);
            break;
        case 2:
            ret = QString::number(item.channel);
            break;
        case 3:
            ret = CanMessage::formatId(item.id);
            break;
        case 4:
            ret = QString::number(item.dlc);
            break;
        case 5:
            if (msg != nullptr) {
                ret = msg->name;
            }
            break;
        case 6:
            ret = "";
            for (int i = 0; i < item.dlc; i++) {
                ret.append(QString(" %1")
                                   .arg(item.data[i], 2, 16, QLatin1Char('0'))
                                   .toUpper());
            }
            break;
        default:
            return {};
        }
    } else {
        auto item = static_cast<CanLogMsg *>(index.internalPointer());
        msg = db.findMessage(item->id);
        auto signal = msg->canSignals.at(index.row());
        switch (index.column()) {
        case 0:
            ret = signal.name;
            break;
        case 1:
            ret = signal.unit;
            break;
        case 2:
            ret = QString("%1").arg(
                    CanSignal::parseSignal(signal, item->data, item->dlc));
            break;
        case 3:
            auto value = CanSignal::parseSignal(signal, item->data, item->dlc);
            ret = signal.display(value);
            break;
        }
    }
    return ret;
}

QVariant CanLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (index.row() >= buffer.size())
        return {};

    auto msg = buffer.at(index.row());

    if (role == Qt::DisplayRole) {
        return displayRowData(index);
    } else if (role == Qt::BackgroundRole) {
        if (rowList.value(index.row(), false)) {
            auto color = QColor(Qt::yellow);
            return QBrush(color);
        } else if (idList.value(msg.id, false)) {
            auto color = QColor(Qt::green);
            return QBrush(color);
        } else {
            auto canmsg = db.findMessage(msg.id);
            if (canmsg != nullptr) {
                return canmsg->color;
            }
            return {};
        }
    } else
        return {};
}

QVariant CanLogModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
    static QVector<QString> const headers = { tr("Number"),
                                              tr("Time"),
                                              tr("Chan"),
                                              tr("CAN ID"),
                                              tr("DLC"),
                                              tr("Message name"),
                                              tr(" 00 01 02 03 04 05 06 07") };

    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal) {
        if (section >= headers.size())
            return {};
        else
            return headers.at(section);
    } else
        return {};
}

void CanLogModel::setHighlightMsg(const QModelIndex &index, bool status)
{
    if (!index.isValid())
        return;

    rowList[index.row()] = status;
}
void CanLogModel::setHighlightId(const QModelIndex &index, bool status)
{
    if (!index.isValid())
        return;
    auto msg = buffer.at(index.row());
    idList[msg.id] = status;
}

bool CanLogModel::isMsgHighlight(const QModelIndex &index)
{
    if (!index.isValid())
        return false;

    return rowList.value(index.row(), false);
}

bool CanLogModel::isIdHighlight(const QModelIndex &index)
{
    if (!index.isValid())
        return false;
    auto msg = buffer.at(index.row());
    return idList.value(msg.id, false);
}

QModelIndex CanLogModel::index(int row, int column,
                               const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (parent.isValid()) {
        auto *log = &buffer[parent.row()];
        auto msg = db.findMessage(log->id);
        if (msg == nullptr) {
            return {};
        }
        if (row < msg->signalCount()) {
            return createIndex(row, column, log);
        }
    } else {
        return createIndex(row, column);
    }
    return {};
}

QModelIndex CanLogModel::parent(const QModelIndex &) const
{
    return {};
}
