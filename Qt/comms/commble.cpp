#include "commble.h"

#include <QBluetoothLocalDevice>
#include <QDateTime>

CommBLE::CommBLE(QObject *parent)
    : Comm{parent}
{

}

void CommBLE::open(const QString &address)
{
    if(m_Controller != nullptr)
        m_Controller->deleteLater();

    QBluetoothAddress localAddress = getLocalAddress();
    if(localAddress.isNull())
        return; // invalid

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    m_BLEController = new QLowEnergyController(QBluetoothAddress(address), localAddress);
#else
    m_Controller = QLowEnergyController::createCentral(QBluetoothAddress(address), localAddress);
#endif
    connect(m_Controller, &QLowEnergyController::connected, m_Controller, &QLowEnergyController::discoverServices);
    connect(m_Controller, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error), this, &CommBLE::onErrorOccurred);
    connect(m_Controller, &QLowEnergyController::serviceDiscovered, this, &CommBLE::onServiceDiscovered);
    m_Controller->connectToDevice();

}

void CommBLE::close()
{
    if(m_RxTxService != nullptr)
    {
        QLowEnergyDescriptor desc = m_RxTxService->characteristic(m_RxUUID).descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        m_RxTxService->writeDescriptor(desc, QByteArray::fromHex("0000"));
        m_RxTxService->deleteLater();
        m_RxTxService = nullptr;
    }
    if(m_Controller != nullptr)
    {
        m_Controller->disconnectFromDevice();
        m_Controller->deleteLater();
        m_Controller = nullptr;
    }
}

void CommBLE::onServiceDiscovered(const QBluetoothUuid& newService)
{
    bool expected = false;

    if(newService == QBluetoothUuid(QLatin1String("00001000-0000-1000-8991-00805f9b34fb"))) // W800K
        expected = true;
    if(newService.toString().contains("-1a48-11e9-ab14-d663bd873d93", Qt::CaseInsensitive)) // Most of the devices
        expected = true;
    if(expected)
    {
        auto service = m_Controller->createServiceObject(newService);
        m_RxTxService = service;
        // for characteristics (assume no included services)
        connect(service, &QLowEnergyService::stateChanged, this, &CommBLE::onServiceDetailDiscovered);
        service->discoverDetails();
    }
}

void CommBLE::onServiceDetailDiscovered(QLowEnergyService::ServiceState newState)
{
    bool deleteService = true;
    auto service = qobject_cast<QLowEnergyService*>(sender());
    if(newState == QLowEnergyService::InvalidService)
        deleteService = true;
    else if(newState == QLowEnergyService::ServiceDiscovered)
    {
        deleteService = true;
        const QList<QLowEnergyCharacteristic> chars = service->characteristics();
        // delete unused service
        bool isRxUUIDValid = false;
        bool isTxUUIDValid = false;
        QBluetoothUuid TxUUID;

        for(auto it = chars.cbegin(); it != chars.cend(); ++it)
        {
            auto uuid = it->uuid();
            if(!isRxUUIDValid && it->properties().testFlag(QLowEnergyCharacteristic::Notify)
                    && (specialRxUUIDList.contains(uuid) || (uuid.toString().contains("2-1a48-11e9-ab14-d663bd873d93", Qt::CaseInsensitive) && !specialTxUUIDList.contains(uuid))))
            {
                isRxUUIDValid = true;
                m_RxUUID = uuid;
                deleteService = false;
            }
            if(!isTxUUIDValid && it->properties().testFlag(QLowEnergyCharacteristic::Write)
                    && (specialTxUUIDList.contains(uuid) || (uuid.toString().contains("3-1a48-11e9-ab14-d663bd873d93", Qt::CaseInsensitive) && !specialRxUUIDList.contains(uuid))))
            {
                isTxUUIDValid = true;
                TxUUID = uuid;
                deleteService = false;
            }
        }

        if(!deleteService)
        {
            if(isRxUUIDValid && isTxUUIDValid)
            {
                connect(m_RxTxService, &QLowEnergyService::stateChanged, this, &CommBLE::onServiceStateChanged);
                // Rx
                connect(m_RxTxService, QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error), this, &CommBLE::onErrorOccurred);
                connect(m_RxTxService, &QLowEnergyService::characteristicChanged, this, &CommBLE::onDataArrived);
                connect(m_RxTxService, &QLowEnergyService::characteristicRead, this, &CommBLE::onDataArrived); // not necessary
                QLowEnergyDescriptor desc = m_RxTxService->characteristic(m_RxUUID).descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                m_RxTxService->writeDescriptor(desc, QByteArray::fromHex("0100")); // Enable notify
                // Tx
                m_TxCharacteristic = m_RxTxService->characteristic(TxUUID);
                emit stateChanged(true);
                emit showMessage(tr("Device Connected"));
                emit deviceFeature(m_RxTxService->serviceUuid().toString());
            }
        }
        else
        {
            service->deleteLater();
        }
    }
}

void CommBLE::onErrorOccurred()
{
    if(sender() == m_Controller)
        qDebug() << "BLE Controller Error:" << m_Controller->error() << m_Controller->errorString();
    else if(sender() == m_RxTxService)
        qDebug() << "BLE Service Error:" << m_RxTxService->error();
}

void CommBLE::onDataArrived(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    // similar to Comm::onReadyRead()
    Q_UNUSED(characteristic);
    lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
    rxBuffer.append(newValue);
    handlePackets();
    if(!rxBufferCleaner->isActive())
        rxBufferCleaner->start();
}

qint64 CommBLE::write(const QByteArray &data)
{
    if(m_RxTxService != nullptr)
    {
        int t = 0;
        for(int i = 0; i < data.length(); i += maxPacketLen)
        {
            // data.mid() will handle the case where i+maxPacketLen > data.length()
            QTimer::singleShot(t, [ = ] {m_RxTxService->writeCharacteristic(m_TxCharacteristic, data.mid(i, maxPacketLen));});
            t += packetDelayMs;
        }
        return data.length(); // no feedback
    }
    else
        return -1;
}

void CommBLE::onServiceStateChanged(QLowEnergyService::ServiceState newState)
{
    if(newState == QLowEnergyService::InvalidService)
    {
        m_RxTxService->deleteLater();
        m_RxTxService = nullptr;
        if(m_Controller != nullptr)
        {
            m_Controller->disconnectFromDevice();
            m_Controller->deleteLater();
            m_Controller = nullptr;
        }
        emit stateChanged(false);
        emit showMessage(tr("Device Disconnected"));
    }
}

const QList<QBluetoothUuid> CommBLE::specialRxUUIDList =
{
    QBluetoothUuid(QLatin1String("00001000-0000-1000-8992-00805f9b34fb")),
    QBluetoothUuid(QLatin1String("48090001-1a48-11e9-ab14-d663bd873d93")),
};

const QList<QBluetoothUuid> CommBLE::specialTxUUIDList =
{
    QBluetoothUuid(QLatin1String("00001000-0000-1000-8993-00805f9b34fb")),
    QBluetoothUuid(QLatin1String("48090002-1a48-11e9-ab14-d663bd873d93")),
};
