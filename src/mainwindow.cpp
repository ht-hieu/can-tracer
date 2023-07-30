#include <array>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QShortcut>
#include <QVector>
#include <QMenu>
#include <QtConcurrent>
#include <QFuture>
#include <QInputDialog>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QGraphicsLayout>
#include <QValueAxis>
#include "logparser.h"
#include "canlogmodel.h"
#include "dbcparser.h"
#include "signalselectdialog.h"
#include "customqchartview.h"

template<typename T>
void resizeColumns(T view)
{
    for (int i = 0; i < view->model()->columnCount(); i++) {
        view->resizeColumnToContents(i);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      msgDb({}),
      log({}),
      model(log, msgDb, this),
      proxyModel(this),
      msgModel(msgDb, this),
      plotModel(this),
      signalModel(msgDb, this),
      colorDelegate(this),
      logDelegate(this)
{
    ui->setupUi(this);

    ui->tblLog->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tblLog->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->viewSignalPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->viewSignalPlot->setModel(&plotModel);
    proxyModel.setSourceModel(&model);
    ui->tblLog->setModel(&proxyModel);
    ui->tblLog->setItemDelegate(&logDelegate);
    ui->tblLog->show();
    ui->viewSignal->setModel(&signalModel);
    resizeColumns(ui->tblLog);

    ui->viewMsg->setItemDelegate(&colorDelegate);
    ui->viewMsg->setModel(&msgModel);
    ui->viewMsg->show();
    resizeColumns(ui->viewMsg);

    ui->graphlayout->setContentsMargins(0, 0, 0, 0);
    ui->graphlayout->setSpacing(0);

    ui->tblLog->setUniformRowHeights(true);
    ui->viewMsg->setUniformRowHeights(true);
    ui->viewSignalPlot->setUniformItemSizes(true);

    connect(ui->btnAddXTick, SIGNAL(clicked()), this, SLOT(onAddXTick()));
    connect(ui->btnAddYTick, SIGNAL(clicked()), this, SLOT(onAddYTick()));
    connect(ui->btnRemXTick, SIGNAL(clicked()), this, SLOT(onRemXTick()));
    connect(ui->btnRemYTick, SIGNAL(clicked()), this, SLOT(onRemYTick()));
    connect(ui->btnZoomInX, SIGNAL(clicked()), this, SLOT(onZoomInX()));
    connect(ui->btnZoomInY, SIGNAL(clicked()), this, SLOT(onZoomInY()));
    connect(ui->btnZoomOutX, SIGNAL(clicked()), this, SLOT(onZoomOutX()));
    connect(ui->btnZoomOutY, SIGNAL(clicked()), this, SLOT(onZoomOutY()));
    connect(ui->btnResetAll, SIGNAL(clicked()), this, SLOT(onResetAllAxis()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->btnOpen, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui->actionOpenDbc, SIGNAL(triggered()), this, SLOT(openDbcFile()));
    connect(ui->btnOpenDbc, SIGNAL(clicked()), this, SLOT(openDbcFile()));
    connect(ui->tblLog, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(onContextMenu(const QPoint &)));
    connect(ui->tblLog->header(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onHeaderContextMenu(const QPoint &)));
    connect(ui->viewSignalPlot, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onSignalPlotMenu(const QPoint &)));
    connect(&logWatcher, &decltype(logWatcher)::finished, this,
            &MainWindow::onLoadLogFile);
    connect(&dbcWatcher, &decltype(dbcWatcher)::finished, this,
            &MainWindow::onLoadDbcFile);
    connect(ui->viewMsg, SIGNAL(clicked(QModelIndex)), this,
            SLOT(onMsgSelect(QModelIndex)));
}

void MainWindow::updateChartAxis()
{
    for (int i = 0; i < plotModel.rowCount(); i++) {
        auto *chartView = plotModel.getChartAt(i);
        chartView->setMinimumWidth(static_cast<int>(time * widthPerSec));
        chartView->setMinimumHeight(minChartSize);
        auto *chart = chartView->chart();
        auto ax = chart->axes(Qt::Horizontal);
        dynamic_cast<QValueAxis *>(ax[0])->setTickCount(
                static_cast<int>(time * tickPerSec));
        ax = chart->axes(Qt::Vertical);
        dynamic_cast<QValueAxis *>(ax[0])->setTickCount(yTick);
    }
}

MainWindow::~MainWindow()
{
    if (dbcFuture.isRunning()) {
        dbcFuture.cancel();
    }
    if (logFuture.isRunning()) {
        logFuture.cancel();
    }
};

// Slots
void MainWindow::openFile()
{
    auto fileName = QFileDialog::getOpenFileName(
            this, tr("Open Log File"), QDir::homePath(),
            tr("Log Files (*.asc *.trc *.blf)"));
    if (fileName == "") {
        return;
    }
    if (logFuture.isRunning()) {
        logFuture.cancel();
    }
    ui->lineLogPath->setText(fileName);
    logFuture = QtConcurrent::run(
            [fileName, this]() { return std::move(Parser::parse(fileName)); });
    logWatcher.setFuture(logFuture);
    ui->statusbar->showMessage(tr("Loading..."));
}

void MainWindow::openDbcFile()
{
    auto fileNames = QFileDialog::getOpenFileNames(this, tr("Open Dbc File"),
                                                   QDir::homePath(),
                                                   tr("Dbc Files (*.dbc)"));
    QString display = "";
    for (auto &name : fileNames) {
        display += name + ";";
    }
    ui->lineDbcPath->setText(display);
    dbcFuture = QtConcurrent::run(
            [fileNames, this]() { return DbcParser::parse(fileNames); });
    dbcWatcher.setFuture(dbcFuture);
}

void MainWindow::onContextMenu(const QPoint &point)
{
    auto index = ui->tblLog->indexAt(point);
    if (index.isValid() && !index.parent().isValid()) {
        auto menu = new QMenu(this);

        QAction *msgAction; // NOLINT
        QAction *idAction; // NOLINT

        auto sourceIndex = proxyModel.mapToSource(index);
        if (model.isMsgHighlight(sourceIndex)) {
            msgAction = new QAction(tr("Unhighlight this message"), this);
            connect(msgAction, &QAction::triggered, this,
                    [sourceIndex, this]() {
                        model.setHighlightMsg(sourceIndex, false);
                    });
        } else {
            msgAction = new QAction(tr("Highlight this message"), this);
            connect(msgAction, &QAction::triggered, this,
                    [sourceIndex, this]() {
                        model.setHighlightMsg(sourceIndex, true);
                    });
        }
        if (model.isIdHighlight(sourceIndex)) {
            idAction = new QAction(tr("Unhighlight this id"), this);
            connect(idAction, &QAction::triggered, this, [sourceIndex, this]() {
                model.setHighlightId(sourceIndex, false);
            });
        } else {
            idAction = new QAction(tr("Highlight this id"), this);
            connect(idAction, &QAction::triggered, this, [sourceIndex, this]() {
                model.setHighlightId(sourceIndex, true);
            });
        }

        menu->addAction(msgAction);
        menu->addAction(idAction);
        menu->popup(ui->tblLog->viewport()->mapToGlobal(point));
    }
}

void MainWindow::onHeaderContextMenu(const QPoint &point)
{
    auto index = ui->tblLog->header()->logicalIndexAt(point);
    bool ok = false;
    auto text = QInputDialog::getText(this, tr("Filter regex"), tr("Regex"),
                                      QLineEdit::Normal,
                                      proxyModel.getFilterText(index), &ok);

    if (ok) {
        proxyModel.setFilter(index, text);
    }
}

void MainWindow::onSignalPlotMenu(const QPoint &point)
{
    if (msgDb.messageCount() == 0) {
        return;
    }
    auto index = ui->viewSignalPlot->indexAt(point);
    auto menu = new QMenu(this);
    auto addAction = new QAction(tr("Add signal"), menu);
    connect(addAction, &QAction::triggered, this,
            [index, this]() { onAddSignal(index); });
    if (index.isValid()) {
        auto removeAction = new QAction(tr("Remove signal"), menu);
        connect(removeAction, &QAction::triggered, this,
                [index, this]() { onRemoveSignal(index); });
        menu->addAction(removeAction);
    }
    menu->addAction(addAction);
    menu->popup(ui->viewSignalPlot->viewport()->mapToGlobal(point));
}

void MainWindow::onLoadLogFile()
{
    ui->statusbar->clearMessage();
    log = std::move(logFuture.result());
    model.logChanged();
    resizeColumns(ui->tblLog);
    plotModel.reset();
    if (log.size() != 0) {
        auto delta = abs(log[0].time - log[log.size() - 1].time);
        time = delta;
    }
}

void MainWindow::onLoadDbcFile()
{
    msgDb = dbcFuture.result();
    model.dbChanged();
    msgModel.dbChanged();
    signalModel.dbChanged();
    resizeColumns(ui->viewMsg);
}

void MainWindow::onMsgSelect(QModelIndex index)
{
    if (index.isValid()) {
        signalModel.setMsgIndex(index.row());
        resizeColumns(ui->viewSignal);
    }
}

void MainWindow::onAddSignal(QModelIndex)
{
    auto dialog = SignalSelectDialog(msgDb, this);
    dialog.exec();
    auto pair = dialog.getResultIndex();
    auto msg = msgDb.at(pair.first);
    auto signal = msg.canSignals.at(pair.second);
    auto data = CanSignal::getSignalGraph(signal, msg.id, log);
    /* These pointers will be free with the chart widget */
    auto *series = new QLineSeries();
    if (data.size() < 3) {
        // Draw all
        for (auto d : data) {
            series->append(d.first, d.second);
        }
    } else {
        // Only draw important points
        int index = 0;
        series->append(data[index].first, data[index].second);
        for (index = 1; index < data.size() - 1; index++) {
            if ((data[index].second != data[index - 1].second)
                || (data[index].second != data[index + 1].second)) {
                series->append(data[index].first, data[index].second);
            }
        }
        series->append(data[index].first, data[index].second);
    }
    auto *chart = new QChart();
    chart->legend()->hide();
    chart->setTitle(signal.name);
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);
    auto ax = chart->axes(Qt::Horizontal, series);
    dynamic_cast<QValueAxis *>(ax[0])->setTickCount(
            static_cast<int>(time * tickPerSec));
    ax = chart->axes(Qt::Vertical, series);
    dynamic_cast<QValueAxis *>(ax[0])->setTickCount(yTick);
    auto *chartView = new CustomQChartView(chart, signal);
    connect(chartView, SIGNAL(pointNotify(QString)), this,
            SLOT(onPointNotify(QString)));
    chartView->setMinimumWidth(static_cast<int>(time * widthPerSec));
    chartView->setMinimumHeight(minChartSize);
    chartView->setContentsMargins(0, 0, 0, 0);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    ui->graphlayout->addWidget(chartView);
    plotModel.addItem(msg.id, signal, chartView);
}

void MainWindow::onPointNotify(QString label)
{
    ui->statusbar->showMessage(label);
}

void MainWindow::onRemoveSignal(QModelIndex index)
{
    plotModel.removeItem(index);
}

void MainWindow::onAddXTick()
{
    tickPerSec += tickPerSecStep;
    updateChartAxis();
}

void MainWindow::onAddYTick()
{
    yTick++;
    updateChartAxis();
}

void MainWindow::onRemXTick()
{
    if (tickPerSec > tickPerSecStep) {
        tickPerSec -= tickPerSecStep;
    }
    updateChartAxis();
}

void MainWindow::onRemYTick()
{
    if (yTick > 1) {
        yTick--;
    }
    updateChartAxis();
}

void MainWindow::onZoomInX()
{
    widthPerSec++;
    updateChartAxis();
}

void MainWindow::onZoomInY()
{
    minChartSize += minChartSizeStep;
    updateChartAxis();
}

void MainWindow::onZoomOutX()
{
    if (widthPerSec > 1) {
        widthPerSec--;
    }
    updateChartAxis();
}

void MainWindow::onZoomOutY()
{
    if (minChartSize > minChartSizeStep) {
        minChartSize -= minChartSizeStep;
    }
    updateChartAxis();
}

void MainWindow::onResetAllAxis()
{
    minChartSize = defaultMinChartSize;
    yTick = defaultYTick;
    tickPerSec = defaultTickPerSec;
    widthPerSec = defaultWidthPerSec;
    updateChartAxis();
}
