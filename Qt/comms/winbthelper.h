#ifndef WINBTHELPER_H
#define WINBTHELPER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>

#ifdef Q_OS_WIN
#include <windows.h>
#include <bluetoothapis.h>
// This should work with MSVC, and it should be ignored when using MinGW compiler.
#pragma comment(lib, "Bthprops.lib")
#endif

// This class only work for Bluetooth Classic devices, because the Qt version is slow.

class WinBTHelper : public QObject
{
    Q_OBJECT
public:
    explicit WinBTHelper(QObject *parent = nullptr);
public slots:
    void start();
signals:
    void deviceDiscovered(QBluetoothDeviceInfo info);
    void finished();
};

#endif // WINBTHELPER_H
