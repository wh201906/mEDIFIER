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
public slots:
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
    QBluetoothUuid m_RxUuid;
    QLowEnergyCharacteristic m_TxCharacteristic;
    const QList<QBluetoothUuid> specialRxUuidList =
    {
        QBluetoothUuid(QLatin1String("00001000-0000-1000-8992-00805f9b34fb")),
        QBluetoothUuid(QLatin1String("48090001-1a48-11e9-ab14-d663bd873d93")),
    };
    const QList<QBluetoothUuid> specialTxUuidList =
    {
        QBluetoothUuid(QLatin1String("00001000-0000-1000-8993-00805f9b34fb")),
        QBluetoothUuid(QLatin1String("48090002-1a48-11e9-ab14-d663bd873d93")),
    };
};

#endif // COMMBLE_H
