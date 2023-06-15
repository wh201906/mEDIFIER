#ifndef DEVICEFORM_H
#define DEVICEFORM_H

#include <QWidget>
#include <QBluetoothDeviceDiscoveryAgent>

namespace Ui
{
class DeviceForm;
}

class DeviceForm : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceForm(QWidget *parent = nullptr);
    ~DeviceForm();
public slots:
    void onCommStateChanged(bool connected);
private slots:
    void onSearchButtonClicked();

    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onDiscoverFinished();
    void onDeviceTableCellClicked(int row, int column);
    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

    void on_searchStopButton_clicked();

private:
    Ui::DeviceForm *ui;

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    bool m_isCurrDiscoveryMethodBLE = false;
    QStringList m_shownDevices;
#ifdef Q_OS_ANDROID
    void getBondedTarget(bool isBLE);
#endif
signals:
    void connectTo(const QString& address, bool isBLE);
    void disconnectDevice();
};

#endif // DEVICEFORM_H
