#include "signalplotlistmodel.h"

SignalPlotListModel::SignalPlotListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant SignalPlotListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (role == Qt::DisplayRole) {
        auto item = items.at(index.row());
        return QString("%2 - %1")
                .arg(CanMessage::formatId(item.msgId))
                .arg(item.signal.name);
    } else
        return {};
}

void SignalPlotListModel::addItem(uint32_t msgId, const CanSignal &signal,
                                  CustomQChartView *widget)
{
    SignalPlotItem const item(msgId, signal);
    auto index = QAbstractItemModel::createIndex(items.size(), 0);
    beginInsertRows(index, items.size(), items.size());
    items.append(item);
    widgets.append(widget);
    endInsertRows();
}

void SignalPlotListModel::removeItem(const QModelIndex &index)
{
    if (index.isValid()) {
        beginRemoveRows(index, index.row(), index.row());
        items.remove(index.row());
        auto widget = widgets.at(index.row());
        delete widget;
        widgets.remove(index.row());
        endRemoveRows();
    }
}

void SignalPlotListModel::reset()
{
    beginResetModel();
    items.clear();
    for (auto w : widgets) {
        delete w;
    }
    widgets.clear();
    endResetModel();
}
