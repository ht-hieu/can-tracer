#ifndef CUSTOMQCHARTVIEW_H
#define CUSTOMQCHARTVIEW_H

#include <QChartView>
#include <QXYSeries>
#include "canmsg.h"

class CustomQChartView : public QChartView
{
    Q_OBJECT
public:
    CustomQChartView(QChart *chart, const CanSignal &signal,
                     QWidget *parent = nullptr);
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *) override;

signals:
    void pointNotify(QString title);

private:
    int index{ -1 };
    int prevIndex{ -1 };
    int startSearchIndex(qreal x);
    int searchNextIndex(qreal x);
    QGraphicsLineItem line;
    CanSignal signal;
    QHash<QXYSeries::PointConfiguration, QVariant> conf;
};

#endif // CUSTOMQCHARTVIEW_H
