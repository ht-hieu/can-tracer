#include "canmsg.h"

constexpr uint8_t bitsInByte = 8;

double CanSignal::parseSignal(const CanSignal &signal, const CanData &input, uint8_t dlc)
{
    auto len = signal.len;
    auto startBit = signal.startBit;
    if (((startBit + len) > (dlc * bitsInByte)) || (signal.len == 0)) {
        return qQNaN();
    }
    QVector<uint8_t> dataBytes;
    while (len) {
        auto remain = startBit % bitsInByte;
        if (remain == 0) {
            if (len >= bitsInByte) {
                dataBytes.append(input.at(startBit / bitsInByte));
                startBit += bitsInByte;
                len -= bitsInByte;
            } else {
                dataBytes.append(GetNbit(input.at(startBit / bitsInByte), len));
                len = 0;
            }
        } else {
            auto bit2Get = bitsInByte - remain;
            if (bit2Get < len) {
                dataBytes.append(
                        GetNbit(input.at(startBit / bitsInByte) >> remain, bit2Get));
                startBit += bit2Get;
                len -= bit2Get;
            } else {
                dataBytes.append(
                        GetNbit(input.at(startBit / bitsInByte) >> remain, len));
                len = 0;
            }
        }
    }

    uint64_t raw = 0;
    if (signal.isBigEndian) {
        for (const uint8_t d : dataBytes) {
            raw <<= bitsInByte;
            raw += d;
        }
    } else {
        auto shiftCnt = 0;
        for (const uint8_t d : dataBytes) {
            raw += d << shiftCnt;
            shiftCnt += bitsInByte;
        }
    }
    double ret = 0;
    if (signal.isSigned) {
        auto signBit = raw >> (signal.len - 1);
        if (signBit) {
            uint64_t mask = UINT64_MAX;
            if (signal.len != 64) {
                mask = (((uint64_t)1) << signal.len) - 1;
            }
            ret = ((int64_t)(raw - mask - 1));
        } else {
            ret = (raw);
        }
    } else {
        ret = static_cast<double>(raw);
    }
    return (ret * signal.scale) + signal.offset;
}

QVector<QPair<double, double>>
CanSignal::getSignalGraph(const CanSignal &signal, uint32_t id,
                          const QVector<CanLogMsg> &log)
{
    /* Include first and last timestamp to make sure all graph have the same
     * x-axis range */
    QVector<QPair<double, double>> ret;
    auto firstTime = log.at(0).time;
    auto lastTime = firstTime;
    double lastValue = 0;
    auto isFirst = true;
    for (auto& data : log) {
        if (data.id == id) {
            auto value = parseSignal(signal, data.data, data.dlc);
            if (isFirst) {
                isFirst = false;
                ret.append({ firstTime, value });
            }
            ret.append({ data.time, value });
            lastValue = value;
        }
        lastTime = data.time;
    }
    if ((log.size() != 0) && (log.end()->id != id)) {
        ret.append({ lastTime, lastValue });
    }
    return ret;
}
