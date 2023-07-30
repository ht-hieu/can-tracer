#include <QFile>
#include <array>
#include <QString>
#include "blfparser.h"

constexpr uint8_t signatureSize = 4;
constexpr uint8_t timeSize = 8;
constexpr uint8_t headerDataSize = 72;
constexpr uint8_t objHeaderSize = 16;
constexpr uint8_t containerHeaderSize = 16;
constexpr uint8_t logContainer = 10;
constexpr uint8_t noCompress = 0;
constexpr uint8_t zlibDeflate = 2;
constexpr uint8_t canMsg = 1;
constexpr uint8_t canMsg2 = 86;
constexpr uint8_t canErrExt = 73;
constexpr uint8_t canFd = 100;
constexpr uint8_t canFd64 = 101;

QDateTime BlfParser::getDateTime(QDataStream &stream)
{
    std::array<quint16, timeSize> raw{};
    for (auto &r : raw) {
        stream >> r;
    }
    return { QDate(raw[0], raw[1], raw[3]),
             QTime(raw[4], raw[5], raw[6], raw[7]) };
}

int BlfParser::parseObject(const QByteArray &bytes,
                           QVector<CanLogMsg> &messages, QByteArray &remain)
{
    auto index = bytes.indexOf("LOBJ");
    if (bytes.size() < 16) {
        // Not enough data to parse, continue at the next object
        remain = bytes;
        return bytes.size();
    }
    if (index == -1) {
        qInfo() << "LOBJ not found 1";
        return -1;
    }

    QDataStream in(bytes);
    in.setByteOrder(QDataStream::LittleEndian);
    in.skipRawData(index + 4);

    qint16 headerSize = 0;
    in >> headerSize;
    qint16 headerVersion = 0;
    in >> headerVersion;
    quint32 objSize = 0;
    in >> objSize;
    quint32 objType = 0;
    in >> objType;
    auto nextPos = index + objSize;
    if (nextPos > bytes.size()) {
        // TODO: What is happening
        remain = bytes;
        return bytes.size();
        // return -1;
    } else {
        remain = {};
    }

    quint32 flags = 0;
    in >> flags;
    if (headerVersion == 1) {
        quint16 dummy = 0;
        in >> dummy; // Client index
        in >> dummy; // Object version
    } else if (headerVersion == 2) {
        quint8 dummy8 = 0;
        in >> dummy8; // Timestamp status
        in >> dummy8; // reserved
        quint16 dummy16 = 0;
        in >> dummy16; // Object version
    }
    quint64 timestamp = 0;
    in >> timestamp;
    if (headerVersion == 2) {
        quint8 dummy = 0;
        for (int i = 0; i < 8; i++) {
            in >> dummy;
        }
    }

    auto factor = (flags == 1) ? 1e-5 : 1e-9;
    if ((objType == canMsg) || (objType == canMsg2)) {
        CanLogMsg msg;
        msg.number = counter++;
        msg.time = factor * timestamp;
        quint16 channel = 0;
        in >> channel;
        msg.channel = channel;
        quint8 flags = 0;
        in >> flags;
        quint8 dlc = 0;
        in >> dlc;
        msg.dlc = dlc;
        quint32 id = 0;
        in >> id;
        msg.id = id & 0x1FFFFFFF;
        in.readRawData((char *)msg.data.data(), 8);
        messages.append(msg);
    }
    index = bytes.indexOf("LOBJ", nextPos);
    if (index >= 0) {
        return index;
    } else {
        return nextPos + (nextPos % 4);
    }
}

int BlfParser::getObject(QDataStream &in, QVector<CanLogMsg> &messages,
                         QByteArray &remain)
{
    // 0
    QByteArray raw(signatureSize, '\0');
    in.readRawData(raw.data(), signatureSize);
    if (std::strncmp(raw, "LOBJ", signatureSize) != 0) {
        return -1;
    }
    quint16 dummy = 0;
    in >> dummy;
    in >> dummy;
    quint32 objSize = 0;
    in >> objSize;
    quint32 objType = 0;
    in >> objType;
    auto dataSize = objSize - objHeaderSize;
    QByteArray objData(dataSize, '\0');
    in.readRawData(objData.data(), dataSize);
    in.skipRawData(dataSize % 4); // Skip padding
    if (objType != logContainer)
        return 0;

    QDataStream data(objData);
    data.setByteOrder(QDataStream::LittleEndian);
    quint16 method = 0;
    data >> method;
    quint8 dummy8 = 0;
    for (int i = 0; i < 6; i++) {
        data >> dummy8;
    }
    quint32 uncompressSize = 0;
    data >> uncompressSize;
    objData.remove(0, containerHeaderSize);
    bool needParse = true;
    QByteArray containerData;
    if (method == noCompress) {
        containerData = objData;
    } else if (method == zlibDeflate) {
        std::array<char, 4> zlibHeader{};
        zlibHeader[0] = (uncompressSize >> 24) & 0xFF;
        zlibHeader[1] = (uncompressSize >> 16) & 0xFF;
        zlibHeader[2] = (uncompressSize >> 8) & 0xFF;
        zlibHeader[3] = (uncompressSize)&0xFF;
        objData.prepend(zlibHeader.data(), 4);
        containerData = qUncompress(objData);
    } else {
        needParse = false;
    }
    if (needParse) {
        while (containerData.size() != 0) {
            if (remain.size() != 0) {
                containerData.prepend(remain);
            }
            auto ret = parseObject(containerData, messages, remain);
            if (ret > 0) {
                containerData.remove(0, ret);
            } else {
                break;
            }
        }
    }
    return 0;
}

QVector<CanLogMsg> BlfParser::parse(const QString &name)
{
    QVector<CanLogMsg> messages{};
    QFile file(name);
    if (!file.open(QFile::ReadOnly)) {
        throw std::runtime_error("Cannot open file");
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // 0
    char raw[signatureSize];
    in.readRawData(raw, signatureSize);
    if (std::strncmp(raw, "LOGG", signatureSize) != 0) {
        throw std::runtime_error("Unexpected format");
    }
    // 1 - 4
    quint32 headerSize = 0;
    in >> headerSize;
    quint8 dummy = 0;
    // 2 - 8 - app id
    in >> dummy;
    // 3 - 9 - app major
    in >> dummy;
    // 4 - 10 - app minor
    in >> dummy;
    // 5 - 11 - app build
    in >> dummy;
    // 6 - 12 - log major
    in >> dummy;
    // 7 - 13 - log minor
    in >> dummy;
    // 8 - 14 - log build
    in >> dummy;
    // 9 - 15 - log patch
    in >> dummy;
    // 10 - 16 - file size
    quint64 fileSize = 0;
    in >> fileSize;
    // 11 - 24 - uncompress size
    quint64 uncompressSize = 0;
    in >> uncompressSize;
    // 12 - 32
    quint32 countObj = 0;
    in >> countObj;
    // 13 - 36
    quint32 countObjRead = 0;
    in >> countObjRead;
    // 14 - 40
    auto startTime = getDateTime(in);
    // 15 - 56
    auto stopTime = getDateTime(in);
    // 16 - 72 - skipp other data
    in.skipRawData(static_cast<int>(headerSize - headerDataSize));

    QByteArray remain{};
    while (!in.atEnd()) {
        auto ret = getObject(in, messages, remain);
        if (ret != 0)
            break;
    }

    return std::move(messages);
}
