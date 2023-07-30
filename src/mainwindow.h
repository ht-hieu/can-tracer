#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QFuture>
#include <QFutureWatcher>
#include <QThreadPool>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include "canlogmodel.h"
#include "canmsgmodel.h"
#include "customproxymodel.h"
#include "colorlisteditor.h"
#include "signalplotlistmodel.h"
#include "cansignalmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ColorPickDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ColorPickDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent){};
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                          const QModelIndex &) const override
    {
        auto *editor = new ColorListEditor(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        auto *colorList = dynamic_cast<ColorListEditor *>(editor);
        colorList->setColor(qvariant_cast<QColor>(
                index.model()->data(index, Qt::EditRole)));
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        auto *colorList = dynamic_cast<ColorListEditor *>(editor);
        auto value = colorList->color();
        model->setData(index, value, Qt::EditRole);
    }
};

class LogDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    static constexpr int colExpand = 300;
    LogDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent){};
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        static const std::array column{ 300, 0, 50, 200 };
        if (index.parent().isValid()) {
            auto newOption = option;
            int offset = 0;
            for (int i = 0; i < index.column() && i < column.size(); i++) {
                offset += column.at(i);
            }
            if (index.column() < column.size()) {
                newOption.rect =
                        QRect(option.rect.x() + offset, option.rect.y(),
                              option.rect.width() + column.at(index.column()),
                              option.rect.height());
            } else {
                newOption.rect =
                        QRect(option.rect.x() + offset, option.rect.y(),
                              option.rect.width(), option.rect.height());
            }
            QStyledItemDelegate::paint(painter, newOption, index);
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void openFile();
    void openDbcFile();
    void onContextMenu(const QPoint &point);
    void onHeaderContextMenu(const QPoint &point);
    void onSignalPlotMenu(const QPoint &point);
    void onMsgSelect(QModelIndex index);
    void onLoadLogFile();
    void onLoadDbcFile();
    void onAddSignal(QModelIndex index);
    void onRemoveSignal(QModelIndex index);
    void onPointNotify(QString label);
    void onAddXTick();
    void onAddYTick();
    void onRemXTick();
    void onRemYTick();
    void onZoomInX();
    void onZoomInY();
    void onZoomOutX();
    void onZoomOutY();
    void onResetAllAxis();

private:
    static constexpr int minChartSizeStep = 5;
    static constexpr double tickPerSecStep = 0.01;
    static constexpr int defaultMinChartSize = 150;
    static constexpr int defaultYTick = 5;
    static constexpr double defaultTickPerSec = 0.1;
    static constexpr int defaultWidthPerSec = 10;

    std::unique_ptr<Ui::MainWindow> ui;
    CanDb msgDb;
    QVector<CanLogMsg> log;
    CanLogModel model;
    CustomProxyModel proxyModel;
    CanMsgModel msgModel;
    SignalPlotListModel plotModel;
    CanSignalModel signalModel;
    ColorPickDelegate colorDelegate;
    LogDelegate logDelegate;
    QFuture<QList<CanLogMsg>> logFuture;
    QFuture<CanDb> dbcFuture;
    QFutureWatcher<QList<CanLogMsg>> logWatcher;
    QFutureWatcher<CanDb> dbcWatcher;
    double time{ 0 };
    int minChartSize{ defaultMinChartSize };
    int yTick{ defaultYTick };
    double tickPerSec{ defaultTickPerSec };
    int widthPerSec{ defaultWidthPerSec };
    void updateChartAxis();
};
#endif // MAINWINDOW_H
