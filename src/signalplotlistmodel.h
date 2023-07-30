#pragma once
#include <QList>
#include <QAbstractListModel>
#include "customqchartview.h"
#include "canmsg.h"

class SignalPlotItem
{
public:
    SignalPlotItem(uint32_t id, const CanSignal &signal)
        : msgId(id), signal(signal)
    {
    }
    uint32_t msgId;
    CanSignal signal;
};

class SignalPlotListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SignalPlotListModel(QObject *parent = nullptr);
    ~SignalPlotListModel()
    {
        for (auto w : widgets) {
            delete w;
        }
    }

    int rowCount([[maybe_unused]] const QModelIndex &parent =
                         QModelIndex()) const override
    {
        return items.size();
    }
    QVariant data(const QModelIndex &index, int role) const override;
    void addItem(uint32_t msgId, const CanSignal &signal,
                 CustomQChartView *widget);
    auto *getChartAt(int index) { return widgets.at(index); }
    void removeItem(const QModelIndex &index);
    void reset();

private:
    QList<SignalPlotItem> items;
    QList<CustomQChartView *> widgets;
};
