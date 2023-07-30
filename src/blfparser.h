#pragma once
#include <QVector>
#include <QDateTime>
#include "canmsg.h"

class BlfParser
{
public:
    QDateTime getDateTime(QDataStream &stream);
    int parseObject(const QByteArray &in, QVector<CanLogMsg> &messages, QByteArray& remain);
    int getObject(QDataStream &stream, QVector<CanLogMsg> &messages, QByteArray& remain);
    QVector<CanLogMsg> parse(const QString &name);

private:
    quint32 counter{ 0 };
};
