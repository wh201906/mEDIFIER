#include "winbthelper.h"

#include <QDebug>

WinBTHelper::WinBTHelper(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<QBluetoothDeviceInfo>();
}

void WinBTHelper::start()
{
#ifdef Q_OS_WIN
    BLUETOOTH_DEVICE_INFO deviceInfo;
    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams;
    searchParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
    searchParams.fReturnAuthenticated = TRUE;
    searchParams.fReturnRemembered = TRUE;
    searchParams.fReturnConnected = TRUE;
    searchParams.fReturnUnknown = TRUE;
    searchParams.fIssueInquiry = TRUE;
    searchParams.cTimeoutMultiplier = 1;

    HBLUETOOTH_DEVICE_FIND hDeviceFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
    if(hDeviceFind == NULL)
    {
        emit finished();
        return;
    }

    do
    {
        QBluetoothDeviceInfo info(QBluetoothAddress((quint64)deviceInfo.Address.ullLong), QString::fromWCharArray(deviceInfo.szName), (quint32)deviceInfo.ulClassofDevice);
        emit deviceDiscovered(info);
    }
    while(BluetoothFindNextDevice(hDeviceFind, &deviceInfo));

    BluetoothFindDeviceClose(hDeviceFind);
    emit finished();
    return;
#else
    qDebug() << "WinBTHelper::start() only works on Windows";
#endif
}

