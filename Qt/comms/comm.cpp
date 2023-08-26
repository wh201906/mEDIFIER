#include "comm.h"

#include <QDebug>
#include <QBluetoothLocalDevice>
#include <QDateTime>

Comm::Comm(QObject *parent)
    : QObject{parent}
{
    rxBufferCleaner = new QTimer();
    connect(rxBufferCleaner, &QTimer::timeout, this, &Comm::rxBufferCleanTask);
    rxBufferCleaner->setInterval(packetTimeoutMs);
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

void Comm::handlePackets()
{
    while(!rxBuffer.isEmpty())
    {
        int packetLen = getPacketLenInBuffer();
        if(packetLen == 0)
            break;
        QByteArray data = removeCheckSum(rxBuffer.left(packetLen));
        rxBuffer.remove(0, packetLen);
        if(!data.isEmpty())
        {
            qDebug() << "received:" << data.toHex();
            emit newData(data);
        }
        else
        {
            qDebug() << "received unexpected:" << rxBuffer.toHex();
        }
    }
}

void Comm::onReadyRead()
{
    lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
    QByteArray rawData = qobject_cast<QIODevice*>(sender())->readAll();
    rxBuffer.append(rawData);
    handlePackets();
    if(!rxBufferCleaner->isActive())
        rxBufferCleaner->start();
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
                 << "expected:" << expected.toHex()
                 << "received:" << data.toHex();
        return QByteArray();
    }
}

int Comm::getPacketLenInBuffer()
{
    if(rxBuffer[0] != '\xBB' && rxBuffer[0] != '\xCC')
    {
        qDebug() << "error:"
                 << "unexpected head:" << (int)rxBuffer[0]
                 << "data:" << rxBuffer.toHex();
        return 0;
    }
    int expectedLength = (int)rxBuffer[1] + 4;
    if(rxBuffer.length() < expectedLength)
    {
        qDebug() << "packet length error:"
                 << "expected:" << expectedLength
                 << "received:" << rxBuffer.length()
                 << "data:" << rxBuffer.toHex();
        return 0;
    }
    return expectedLength;
}

QBluetoothAddress Comm::getLocalAddress()
{
    QBluetoothAddress localAddress;

    auto BTAdapterList = QBluetoothLocalDevice::allDevices();
    for(auto it = BTAdapterList.cbegin(); it != BTAdapterList.cend(); ++it)
    {
        qDebug() << "dev:" << it->name() << it->address();
        QBluetoothLocalDevice dev(it->address());
        if(dev.isValid() && dev.hostMode() != QBluetoothLocalDevice::HostPoweredOff)
        {
            localAddress = it->address();
            break; // find the first valid one
        }
    }
    return localAddress;
}

void Comm::rxBufferCleanTask()
{
    if(QDateTime::currentMSecsSinceEpoch() - lastReceiveTime >= packetTimeoutMs)
    {
        rxBuffer.clear();
        rxBufferCleaner->stop();
    }
}
