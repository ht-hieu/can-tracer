#pragma once
#include <QString>
#include <QVector>
#include "canmsg.h"

class Parser
{
public:
    static QVector<CanLogMsg> parse(const QString &name);
};
