#pragma once
#include <QString>
#include "canmsg.h"

class DbcParser
{
public:
    static CanDb parse(const QStringList &files);
    static void parseFile(const QString &filename, CanDb &db);
    static void parseStream(QTextStream &in, CanDb &db);
};
