#ifndef COMMBLE_H
#define COMMBLE_H

#include "comm.h"
#include <QLowEnergyController>

class CommBLE : public Comm
{
    Q_OBJECT
public:
    explicit CommBLE(QObject *parent = nullptr);
    void open(const QString& address) override;
    void close() override;
protected:
    qint64 write(const QByteArray &data) override;
private slots:
    void onServiceDiscovered(const QBluetoothUuid &newService);
    void onErrorOccurred();
    void onServiceDetailDiscovered(QLowEnergyService::ServiceState newState);
    void onDataArrived(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void onServiceStateChanged(QLowEnergyService::ServiceState newState);
private:
    QLowEnergyController* m_Controller = nullptr;
    QList<QBluetoothUuid> m_DiscoveredServices;
    QLowEnergyService* m_RxTxService = nullptr;
    QBluetoothUuid m_RxUUID;
    QLowEnergyCharacteristic m_TxCharacteristic;
    static const QList<QBluetoothUuid> specialRxUUIDList;
    static const QList<QBluetoothUuid> specialTxUUIDList;
    static const int maxPacketLen = 20; // MTU?
    static const int packetDelayMs = 50;
};

#endif // COMMBLE_H
