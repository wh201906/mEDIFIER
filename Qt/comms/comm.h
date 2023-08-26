#ifndef COMM_H
#define COMM_H

#include <QObject>
#include <QBluetoothAddress>
#include <QDateTime>

class Comm : public QObject
{
    Q_OBJECT
public:
    explicit Comm(QObject *parent = nullptr);
    virtual void open(const QString& address) = 0;
    virtual void close() = 0;
    int getPacketLenInBuffer();
    static QByteArray addPacketHead(QByteArray cmd);
    static QByteArray addChecksum(QByteArray data);
    static QByteArray removeCheckSum(QByteArray data);
    static QBluetoothAddress getLocalAddress();

    static const int packetTimeoutMs = 5000;
public slots:
    bool sendCommand(const QByteArray& cmd, bool isRaw = false);
    bool sendCommand(const char* hexCmd, bool isRaw = false);
protected:
    virtual qint64 write(const QByteArray &data) = 0;
    void handlePackets();

    QByteArray rxBuffer;
    qint64 lastReceiveTime = 0;
protected slots:
    void onReadyRead();
signals:
    void newData(const QByteArray& data);
    void stateChanged(bool connected);
    void showMessage(const QString& msg);
    void deviceFeature(const QString& feature, bool isBLE = true);
};

#endif // COMM_H
