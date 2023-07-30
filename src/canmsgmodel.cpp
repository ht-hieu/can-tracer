#include <array>
#include <QColor>
#include "canmsgmodel.h"

constexpr uint8_t ColorColumn = 4;

CanMsgModel::CanMsgModel(CanDb &db, QObject *parent) : QAbstractListModel(parent), db(db) { }

int CanMsgModel::rowCount([[maybe_unused]] const QModelIndex &parent) const
{
    return db.messageCount();
}

QVariant CanMsgModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    auto msg = db.at(index.row());

    if (role == Qt::DisplayRole) {
        QString ret = "";
        switch (index.column()) {
        case 0:
            ret = QString("%1")
                          .arg(msg.id,
                               (msg.id > maxNormalCanId) ? extCanNibble
                                                         : normalCanNibble,
                               16, QLatin1Char('0'))
                          .toUpper();
            break;
        case 1:
            ret = msg.name;
            break;
        case 2:
            ret = QString("%1").arg(msg.dlc);
            break;
        case 3:
            ret = msg.sender;
            break;
        default:
            return {};
        }
        return ret;
    } else if (((role == Qt::DecorationRole) || (role == Qt::EditRole))
               && (index.column() == ColorColumn)) {
        return msg.color;
    } else {
        return {};
    }
}

bool CanMsgModel::setData(const QModelIndex &index, const QVariant &value,
                          int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        auto msg = db.at(index.row());
        db.setColor(msg.id, qvariant_cast<QColor>(value));
        emit dataChanged(index, index, { role });
        return true;
    }
    return false;
}

QVariant CanMsgModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
    static std::array<QString, columnCnt> const headers = {
        tr("CAN ID"), tr("Message name"), tr("DLC"), tr("Sender"), tr("Color")
    };

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

QVariant CanMsgModel::getMsgId(QModelIndex index) const
{
    if (!index.isValid())
        return {};

    auto msg = db.at(index.row());
    return msg.id;
}

Qt::ItemFlags CanMsgModel::flags(const QModelIndex &index) const
{
    if (index.isValid() && (index.column() == ColorColumn)) {
        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
    }

    return Qt::ItemIsEnabled;
}
