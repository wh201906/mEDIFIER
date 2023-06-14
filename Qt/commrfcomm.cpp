#include "commrfcomm.h"

#include <QBluetoothLocalDevice>

CommRFCOMM::CommRFCOMM(QObject *parent)
    : Comm{parent}
{
    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    connect(m_socket, &QIODevice::readyRead, this, &CommRFCOMM::onReadyRead);
    connect(m_socket, &QBluetoothSocket::connected, this, &CommRFCOMM::onStateChanged);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &CommRFCOMM::onStateChanged);
}

void CommRFCOMM::open(const QString &address)
{
    m_socket->connectToService(QBluetoothAddress(address), m_serviceUUID);
}

void CommRFCOMM::close()
{
    m_socket->disconnectFromService();
    m_socket->close();
}

qint64 CommRFCOMM::write(const QByteArray &data)
{
    return m_socket->write(data);
}

void CommRFCOMM::onStateChanged()
{
    QBluetoothSocket::SocketState state = m_socket->state();
    if(state == QBluetoothSocket::ConnectedState)
        emit stateChanged(true);
    else if(state == QBluetoothSocket::UnconnectedState)
        emit stateChanged(false);
}
