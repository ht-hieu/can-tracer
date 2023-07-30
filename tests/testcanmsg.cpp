#include <QTest>
#include "canmsg.h"

class TestCanMsg : public QObject
{
    Q_OBJECT
private slots:
    void testCanMessage()
    {
        CanMessage msg;
        CanSignal signal;
        msg.addCanSignal(signal);
        QCOMPARE(msg.signalCount(), 1);
    }

    void testCanDb()
    {
        CanDb db{};
        CanMessage msg;
        db.addMessage(msg);
        QCOMPARE(db.messageCount(), 1);
    }

    void testCanDbParseSignal()
    {
        CanSignal signal(0, 16, true, false, 1, 0);

        CanData data = {0x12, 0x34};
        QCOMPARE(CanSignal::parseSignal(signal, data, 2), 0x1234);
        signal.isBigEndian = false;
        QCOMPARE(CanSignal::parseSignal(signal, data, 2), 0x3412);
        signal.offset = 6;
        QCOMPARE(CanSignal::parseSignal(signal, data, 2), 0x3418);
        signal.scale = 0.5;
        QCOMPARE(CanSignal::parseSignal(signal, data, 2), 0x1A0F);
        data[1] = 0xFF;
        signal.scale = 1;
        signal.offset = 0;
        signal.isSigned = true;
        QCOMPARE(CanSignal::parseSignal(signal, data, 2), -238);
    }
};

QTEST_MAIN(TestCanMsg)
#include "testcanmsg.moc"
