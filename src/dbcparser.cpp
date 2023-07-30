#include "dbcparser.h"
#include <string_view>
#include <QFile>
#include <QRegularExpression>

// Message definition
// BO_ <id> <name>: <dlc> <sender>

// Signal definition
// SG_ <name> : <start bit>|<length>@<endian><signed> (<scale>,<offset>)
// [<min>|<max>] "<unit>" <receiver>

static const std::string_view messagePattern{
    R"(BO_\s+(\d+)\s+([\d\w]+):\s+(\d+) ([\d\w]+))"
};
static const std::string_view commentMessagePattern{
    "CM_\\s+BO_\\s+(\\d+)\\s+\"(.*)\";"
};
static const std::string_view signalPattern{
    R"lit(SG_ (.+) : (\d+)\|(\d+)@(0|1)(\+|\-) \(([\d\.]+),([-\d\.]+)\) \[([-\d\.]+)\|([-\d\.]+)\] "(.*)"\s+([\w\d]*))lit"
};
static const std::string_view commentSignalPattern{
    R"lit(CM_\s+SG_\s+(\d+)\s([\d\w]+)\s+"(.*)";)lit"
};
static const std::string_view valuePattern{
    R"(VAL_\s+(\d+)\s+([\d\w]+)((?:\s+[\d]+\s+".*?")+)\s?;)"
};
static const std::string_view valueDecodePattern{ "\\s+([\\d]+)\\s+\"(.*?)\"" };

constexpr uint8_t canIdOfs = 1;
constexpr uint8_t messageNameOfs = 2;
constexpr uint8_t dlcOfs = 3;
constexpr uint8_t senderOfs = 4;
constexpr uint8_t signalnameOfs = 1;
constexpr uint8_t startBitOfs = 2;
constexpr uint8_t lengthOfs = 3;
constexpr uint8_t endianOfs = 4;
constexpr uint8_t signedOfs = 5;
constexpr uint8_t scaleOfs = 6;
constexpr uint8_t offsetOfs = 7;
constexpr uint8_t minOfs = 8;
constexpr uint8_t maxOfs = 9;
constexpr uint8_t unitOfs = 10;
constexpr uint8_t receiverOfs = 11;

using PARSE_STATE = enum { IDLE, MESSAGE, SIGNAL };

void DbcParser::parseStream(QTextStream &in, CanDb &db)
{
    const QRegularExpression msgRe(messagePattern.data());
    const QRegularExpression signalRe(signalPattern.data());
    const QRegularExpression valRe(valuePattern.data());
    CanMessage message;
    auto state = IDLE;
    while (!in.atEnd()) {
        auto line = in.readLine();
        bool ok = false;
        if (state == IDLE) {
            QString value;
            auto match = msgRe.match(line);
            if (match.hasMatch()) {
                state = MESSAGE;
                value = match.captured(canIdOfs);
                auto id = value.toULong(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                message.id = removeExtMask(id);
                message.name = match.captured(messageNameOfs);
                value = match.captured(dlcOfs);
                message.dlc = value.toShort(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                message.sender = match.captured(senderOfs);
                continue;
            }
            match = valRe.match(line);
            if (match.hasMatch()) {
                value = match.captured(1);
                auto id = removeExtMask(value.toULong(&ok));
                if (!ok) {
                    continue;
                }
                value = match.captured(2);
                auto msg = db.findMessage(id);
                if (msg == nullptr) {
                    continue;
                }
                auto signal = msg->findSignal(value);
                if (signal == nullptr) {
                    continue;
                }
                value = match.captured(3);
                QRegularExpression const valuePair(valueDecodePattern.data());
                auto pairs = valuePair.globalMatch(value);
                while (pairs.hasNext()) {
                    auto pairMatch = pairs.next();
                    value = pairMatch.captured(1);
                    QPair<double, QString> pair;
                    pair.first = value.toDouble(&ok);
                    if (!ok) {
                        continue;
                    }
                    pair.second = pairMatch.captured(2);
                    signal->addValuePair(pair);
                }
            }
        } else if (state == MESSAGE) {
            auto match = signalRe.match(line);
            if (match.hasMatch()) {
                CanSignal signal;
                signal.name = match.captured(signalnameOfs);
                QString value;
                bool ok = false;
                value = match.captured(startBitOfs);
                signal.startBit = value.toUInt(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                value = match.captured(lengthOfs);
                signal.len = value.toUInt(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                value = match.captured(endianOfs);
                signal.isBigEndian = !value.toUInt(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                value = match.captured(signedOfs);
                if (value.compare("-") == 0) {
                    signal.isSigned = true;
                } else {
                    signal.isSigned = false;
                }
                value = match.captured(scaleOfs);
                signal.scale = value.toDouble(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                value = match.captured(offsetOfs);
                signal.offset = value.toDouble(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                value = match.captured(minOfs);
                signal.min = value.toDouble(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                value = match.captured(maxOfs);
                signal.max = value.toDouble(&ok);
                if (!ok) {
                    state = IDLE;
                    continue;
                }
                signal.unit = match.captured(unitOfs);
                signal.receiver = match.captured(receiverOfs);
                message.addCanSignal(signal);
            } else {
                db.addMessage(message);
                message = {};
                state = IDLE;
            }
        }
    }
    if (state == MESSAGE) {
        db.addMessage(message);
    }
}

void DbcParser::parseFile(const QString &filename, CanDb &db)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Cannot open " << filename;
        return;
    }
    QTextStream in(&file);
    parseStream(in, db);
    file.close();
}

CanDb DbcParser::parse(const QStringList &files)
{
    CanDb db{};
    for (const auto &file : files) {
        parseFile(file, db);
    }
    return db; // Success
}
