#include "comm.h"

#include <QDebug>

Comm::Comm(QObject *parent)
    : QObject{parent}
{

}

bool Comm::sendCommand(const QByteArray& cmd, bool isRaw)
{
    qDebug() << "send:" << cmd.toHex();
    if(isRaw)
        return write(cmd);
    else
        return write(addChecksum(addPacketHead(cmd)));
}

bool Comm::sendCommand(const char *hexCmd, bool isRaw)
{
    return sendCommand(QByteArray::fromHex(hexCmd), isRaw);
}

void Comm::onReadyRead()
{
    emit newData(qobject_cast<QIODevice*>(sender())->readAll());
}

QByteArray Comm::addPacketHead(QByteArray cmd)
{
    cmd.prepend(cmd.length()); // [1]
    cmd.prepend('\xAA'); // [0]
    return cmd;
}

QByteArray Comm::addChecksum(QByteArray cmd)
{
    quint16 sum = 8217;
    for(quint8 i : cmd)
        sum += i;
    cmd.append(sum >> 8);
    cmd.append(sum & 0xFF);
    return cmd;
}
