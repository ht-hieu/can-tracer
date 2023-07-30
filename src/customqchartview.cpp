#include <QLineSeries>
#include "customqchartview.h"

CustomQChartView::CustomQChartView(QChart *chart, const CanSignal &signal,
                                   QWidget *parent)
    : QChartView(chart, parent), line(chart), signal(signal)
{
}

int CustomQChartView::startSearchIndex(qreal x)
{
    auto series = dynamic_cast<QLineSeries *>(chart()->series()[0]);

    int count = series->count();
    int step = 0;
    int it = 0;
    int first = 0;

    while (count > 0) {
        it = first;
        step = count / 2;
        it += step;

        if (series->at(it).x() < x) {
            first = ++it;
            count -= step + 1;
        } else
            count = step;
    }
    if (first == count) {
        first = count - 1;
    }
    return first;
}

void CustomQChartView::mouseMoveEvent(QMouseEvent *event)
{
    auto const widgetPos = event->position();
    auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()),
                                            static_cast<int>(widgetPos.y())));
    auto const chartItemPos = chart()->mapFromScene(scenePos);
    auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
    if (chart()->contains(chartItemPos)) {
        auto x = valueGivenSeries.x();
        auto series = dynamic_cast<QLineSeries *>(chart()->series()[0]);
        index = startSearchIndex(x);
        auto point = series->at(index);
        auto label = QString("%3: %1, %2")
                             .arg(x)
                             .arg(signal.display(point.y()))
                             .arg(chart()->title());
        prevIndex = index;
        const QLineF xLine(chartItemPos.x(), chart()->plotArea().top(),
                           chartItemPos.x(), chart()->plotArea().bottom());
        line.setLine(xLine);
        line.show();
        emit pointNotify(label);
    }
}

void CustomQChartView::leaveEvent(QEvent *)
{
    dynamic_cast<QLineSeries *>(chart()->series()[0])
            ->clearPointsConfiguration();
    index = -1;
    prevIndex = -1;
    line.hide();
}
