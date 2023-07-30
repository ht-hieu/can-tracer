#pragma once
#include <optional>
#include <QObject>
#include <QMetaType>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QVector>
#include <QDebug>
#include <QColor>
#include <QPair>

constexpr uint16_t maxNormalCanId = 0x7FF;
constexpr uint8_t normalCanNibble = 3;
constexpr uint8_t extCanNibble = 8;
constexpr uint8_t CAN_MAX_DLC = 8;

using CanData = std::array<uint8_t, CAN_MAX_DLC>;

constexpr uint32_t removeExtMask(uint32_t id)
{
    return id & 0x1F'FF'FF'FF;
}

constexpr uint8_t GetNbit(uint8_t input, uint8_t n)
{
    return (input & ((1 << n) - 1));
}

using CAN_DIR = enum { CAN_DIR_RX = 0, CAN_DIR_TX };

struct CanLogMsg
{
    CanLogMsg() : data({}){};

    bool setDir(const QString &str)
    {
        if (str.compare("Rx")) {
            dir = CAN_DIR_RX;
            return true;
        } else if (str.compare("Tx")) {
            dir = CAN_DIR_TX;
            return true;
        } else {
            return false;
        }
    }

    uint32_t number{ 0 };
    uint32_t id{ 0 };
    double time{ 0.0 };
    uint8_t channel{ 0 };
    uint8_t dlc{ CAN_MAX_DLC };
    uint8_t dir{ CAN_DIR_RX };
    CanData data;
};

struct CanSignal
{
    CanSignal() = default;
    CanSignal(uint8_t startBit, uint8_t len, bool isBigEndian, bool isSigned,
              double scale, double offset)
        : startBit(startBit),
          len(len),
          isBigEndian(isBigEndian),
          isSigned(isSigned),
          scale(scale),
          offset(offset)
    {
    }
    uint8_t startBit;
    uint8_t len;
    bool isBigEndian;
    bool isSigned;
    double scale;
    double offset;
    double min;
    double max;
    QString name;
    QString unit;
    QString receiver;
    QVector<QPair<double, QString>> values{};

    auto &getValuePairs() const { return values; }

    void addValuePair(const QPair<double, QString> &value)
    {
        values.append(value);
    }

    static double parseSignal(const CanSignal &signal, const CanData &input,
                              uint8_t dlc);
    QString display(double value)
    {
        for (auto i : values) {
            if (i.first == value) {
                return i.second;
            }
        }
        return QString("%1").arg(value);
    }

    static QVector<QPair<double, double>>
    getSignalGraph(const CanSignal &signal, uint32_t id,
                   const QVector<CanLogMsg> &log);
};

struct CanMessage
{
    CanMessage() : color(0, 0, 0, 0) { }
    void addCanSignal(const CanSignal &signal) { canSignals.append(signal); }
    auto signalCount() const { return canSignals.size(); }

    static QString formatId(uint32_t id)
    {
        return QString("%1")
                .arg(id, (id > maxNormalCanId) ? extCanNibble : normalCanNibble,
                     16, QLatin1Char('0'))
                .toUpper();
    }

    CanSignal *findSignal(const QString &name)
    {
        for (auto &canSignal : canSignals) {
            if (canSignal.name == name) {
                return &canSignal;
            }
        }
        return nullptr;
    }

    QString formatId() { return formatId(id); }

    uint32_t id;
    uint8_t dlc;
    QString name;
    QString sender;
    QVector<CanSignal> canSignals;
    QColor color;
};

class CanDb
{
public:
    CanDb() : db({}){};
    void addMessage(const CanMessage &msg) { db.append(msg); }
    auto messageCount() const { return db.count(); }
    auto &at(qsizetype i) const { return db.at(i); }

    const CanMessage *findMessage(uint32_t id) const
    {
        for (const auto &i : db) {
            if (i.id == id) {
                return &i;
            }
        }
        return nullptr;
    }

    CanMessage *findMessage(uint32_t id)
    {
        for (auto &i : db) {
            if (i.id == id) {
                return &i;
            }
        }
        return nullptr;
    }

    void setColor(uint32_t id, const QColor &color)
    {
        auto it = findMessage(id);

        if (it != nullptr) {
            it->color = color;
        }
    }

private:
    QVector<CanMessage> db;
};
