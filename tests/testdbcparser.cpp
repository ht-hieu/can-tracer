#include <QTest>
#include <QTextStream>
#include "canmsg.h"

#include "dbcparser.h"

class TestDbcParser : public QObject
{
    Q_OBJECT
private slots:
    void test1()
    {
        QString text =
                "BO_ 123 hello: 7 Bob\n"
                " SG_ hi : 0|8@1+ (1,0) [0|255] \"\" Alice\n";
        QTextStream stream(&text);
        CanDb db;
        DbcParser::parseStream(stream, db);
        QCOMPARE(db.messageCount(), 1);
        QCOMPARE(db.at(0).signalCount(), 1);
    }
};

QTEST_MAIN(TestDbcParser)
#include "testdbcparser.moc"
