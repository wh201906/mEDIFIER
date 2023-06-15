#include "comm.h"

#include <QDebug>
#include <QTimer>

Comm::Comm(QObject *parent)
    : QObject{parent}
{

}

bool Comm::sendCommand(const QByteArray& cmd, bool isRaw)
{
    QByteArray data = cmd;
    if(!isRaw)
        data = addChecksum(addPacketHead(cmd));
    qDebug() << "send:" << data.toHex();
    return write(data);
}

bool Comm::sendCommand(const char *hexCmd, bool isRaw)
{
    return sendCommand(QByteArray::fromHex(hexCmd), isRaw);
}

void Comm::onReadyRead()
{
    // checksum is removed there.
    QByteArray data = checkValidity(qobject_cast<QIODevice*>(sender())->readAll());
    if(!data.isEmpty())
    {
        qDebug() << "received:" << data.toHex();
        emit newData(data);
    }
}

QByteArray Comm::addPacketHead(QByteArray cmd)
{
    cmd.prepend(cmd.length()); // [1]
    cmd.prepend('\xAA'); // [0]
    return cmd;
}

QByteArray Comm::addChecksum(QByteArray data)
{
    quint16 sum = 8217;
    for(quint8 i : data)
        sum += i;
    data.append(sum >> 8);
    data.append(sum & 0xFF);
    return data;
}

QByteArray Comm::removeCheckSum(QByteArray data)
{
    QByteArray removed = data.chopped(2);
    QByteArray expected = addChecksum(removed);
    if(data == expected)
    {
        return removed;
    }
    else
    {
        qDebug() << "checksum error:"
                 << "expected:" << expected
                 << "received:" << data;
        return QByteArray();
    }
}

QByteArray Comm::checkValidity(QByteArray data)
{
    if(data[0] != '\xBB' && data[0] != '\xCC')
    {
        qDebug() << "error:"
                 << "unexpected head:" << (int)data[0]
                 << "data:" << data.toHex();
        return QByteArray();
    }
    int expectedLength = (int)data[1] + 4;
    if(expectedLength != data.length())
    {
        qDebug() << "packet length error:"
                 << "expected:" << expectedLength
                 << "received:" << data.length()
                 << "data:" << data.toHex();
        return QByteArray();
    }
    return removeCheckSum(data);
}
