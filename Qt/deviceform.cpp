#include "deviceform.h"
#include "ui_deviceform.h"
#include "comms/comm.h"

#include <QDebug>
#include <QBluetoothUuid>
#include <QBluetoothLocalDevice>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#endif

DeviceForm::DeviceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceForm)
{
    ui->setupUi(this);

    // the userData there is "isBLE"
    ui->deviceTypeBox->addItem("RFCOMM", false);
    ui->deviceTypeBox->addItem("BLE", true);

    ui->disconnectButton->setVisible(false);
    ui->searchStopButton->setVisible(false);

    ui->deviceTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->searchRFCOMMButton, &QPushButton::clicked, this, &DeviceForm::onSearchButtonClicked);
    connect(ui->searchBLEButton, &QPushButton::clicked, this, &DeviceForm::onSearchButtonClicked);
    connect(ui->deviceTableWidget, &QTableWidget::cellClicked, this, &DeviceForm::onDeviceTableCellClicked);

    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceForm::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceForm::onDiscoverFinished);
#ifdef Q_OS_WIN
    m_winBTThread = new QThread();
    m_winBTHelper = new WinBTHelper();
    connect(this, &DeviceForm::startDiscovery, m_winBTHelper, &WinBTHelper::start);
    connect(m_winBTHelper, &WinBTHelper::deviceDiscovered, this, &DeviceForm::onDeviceDiscovered);
    connect(m_winBTHelper, &WinBTHelper::finished, this, &DeviceForm::onDiscoverFinished);
    m_winBTHelper->moveToThread(m_winBTThread);
    m_winBTThread->start();
#endif
}

DeviceForm::~DeviceForm()
{
    delete ui;
}

void DeviceForm::onSearchButtonClicked()
{
#ifdef Q_OS_ANDROID
    getRequiredPermission();
#endif
    if(Comm::getLocalAddress().isNull())
    {
        emit showMessage(tr("Bluetooth is not available"));
        return;
    }
    if(sender() == ui->searchRFCOMMButton)
        m_isCurrDiscoveryMethodBLE = false;
    else if(sender() == ui->searchBLEButton)
        m_isCurrDiscoveryMethodBLE = true;
    else
    {
        qDebug() << "error: unexpected sender" << sender();
        return;
    }
    ui->deviceTableWidget->setRowCount(0);
    m_shownDevices.clear();
#ifdef Q_OS_ANDROID
    getBondedTarget(m_isCurrDiscoveryMethodBLE);
#endif
#ifdef Q_OS_WIN
    if(m_isCurrDiscoveryMethodBLE)
        m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    else
        emit startDiscovery(); // faster
#else
    m_discoveryAgent->start(m_isCurrDiscoveryMethodBLE ?
                            QBluetoothDeviceDiscoveryAgent::LowEnergyMethod :
                            QBluetoothDeviceDiscoveryAgent::ClassicMethod);
#endif
    ui->searchRFCOMMButton->setVisible(false);
    ui->searchBLEButton->setVisible(false);
    ui->searchStopButton->setVisible(true);

}

void DeviceForm::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    QString address = info.address().toString();
    QString name = info.name();
    if(m_shownDevices.contains(address, Qt::CaseInsensitive))
    {
        qDebug() << "dumplicate:" << address << name;
        return;
    }
    QTableWidget* deviceTable = ui->deviceTableWidget;
    int i;

    i = deviceTable->rowCount();
    deviceTable->setRowCount(i + 1);
    deviceTable->setItem(i, 0, new QTableWidgetItem(name));
    deviceTable->setItem(i, 1, new QTableWidgetItem(address));
    QTableWidgetItem* typeItem = new QTableWidgetItem();

    if(m_isCurrDiscoveryMethodBLE)
        typeItem->setText(tr("BLE"));
    else
        typeItem->setText(tr("RFCOMM"));
    typeItem->setData(Qt::UserRole, m_isCurrDiscoveryMethodBLE);
    deviceTable->setItem(i, 2, typeItem);
    m_shownDevices.append(address);

    qDebug() << name
             << address
             << info.isValid()
             << info.rssi()
             << info.majorDeviceClass()
             << info.minorDeviceClass()
             << info.deviceUuid()
             << info.serviceUuids()
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
             << info.serviceClasses();
#else
             << info.serviceClasses()
             << info.manufacturerData();
#endif
}

void DeviceForm::onDiscoverFinished()
{
    ui->searchRFCOMMButton->setVisible(true);
    ui->searchBLEButton->setVisible(true);
    ui->searchStopButton->setVisible(false);
}

void DeviceForm::onDeviceTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    ui->deviceAddressEdit->setText(ui->deviceTableWidget->item(row, 1)->text());
    bool isBLE = ui->deviceTableWidget->item(row, 2)->data(Qt::UserRole).toBool();
    ui->deviceTypeBox->setCurrentIndex(ui->deviceTypeBox->findData(isBLE));
}

void DeviceForm::onCommStateChanged(bool connected)
{
    ui->connectButton->setVisible(!connected);
    ui->disconnectButton->setVisible(connected);
}

void DeviceForm::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if(m_settings == nullptr)
        return;
    m_settings->beginGroup("DeviceForm");
    ui->deviceAddressEdit->setText(m_settings->value("LastDeviceAddress").toString());
    int lastDeviceTypeIndex = ui->deviceTypeBox->findText(m_settings->value("LastDeviceType").toString());
    if(lastDeviceTypeIndex > -1 && lastDeviceTypeIndex < ui->deviceTypeBox->count())
        ui->deviceTypeBox->setCurrentIndex(lastDeviceTypeIndex);
    m_settings->endGroup();
}

void DeviceForm::on_connectButton_clicked()
{
    QString addressStr = ui->deviceAddressEdit->text();
    if(QBluetoothAddress(addressStr).isNull())
    {
        emit showMessage(tr("Not a valid Bluetooth address"));
        return;
    }
    bool isBLE = ui->deviceTypeBox->currentData().toBool();
    emit connectTo(addressStr, isBLE);
    m_settings->beginGroup("DeviceForm");
    m_settings->setValue("LastDeviceAddress", addressStr);
    m_settings->setValue("LastDeviceType", ui->deviceTypeBox->currentText());
    m_settings->endGroup();
}

void DeviceForm::on_disconnectButton_clicked()
{
    emit disconnectDevice();
}

void DeviceForm::on_searchStopButton_clicked()
{
    m_discoveryAgent->stop();
    onDiscoverFinished();
}

void DeviceForm::setSettings(QSettings* settings)
{
    m_settings = settings;
}

#ifdef Q_OS_ANDROID
bool DeviceForm::getPermission(const QString& permission)
{
    QtAndroid::PermissionResult result = QtAndroid::checkPermission(permission);
    if(result == QtAndroid::PermissionResult::Denied)
    {
        QtAndroid::requestPermissionsSync(QStringList() << permission);
        result = QtAndroid::checkPermission(permission);
        if(result == QtAndroid::PermissionResult::Denied)
            return false;
    }
    return true;
}

void DeviceForm::getRequiredPermission()
{
    QStringList permissionList =
    {
        "android.permission.ACCESS_FINE_LOCATION",
        "android.permission.BLUETOOTH_ADMIN"
    };
    if(QtAndroid::androidSdkVersion() >= 31)
    {
        permissionList += "android.permission.BLUETOOTH_SCAN";
        permissionList += "android.permission.BLUETOOTH_CONNECT";
    }
    else
    {
        permissionList += "android.permission.BLUETOOTH";
    }
    for(const QString& permission : permissionList)
    {
        if(!getPermission(permission))
            qDebug() << "Failed to request permission" << permission;
    }
}

void DeviceForm::getBondedTarget(bool isBLE)
{
    QAndroidJniEnvironment androidEnv;
    getRequiredPermission();
    QAndroidJniObject array = QtAndroid::androidActivity().callObjectMethod("getBondedDevices", "(Z)[Ljava/lang/String;", isBLE);
    int arrayLen = androidEnv->GetArrayLength(array.object<jarray>());
    qDebug() << "arrayLen:" << arrayLen;
    QTableWidget* deviceTable = ui->deviceTableWidget;
    deviceTable->setRowCount(arrayLen);
    for(int i = 0; i < arrayLen; i++)
    {
        QString info = QAndroidJniObject::fromLocalRef(androidEnv->GetObjectArrayElement(array.object<jobjectArray>(), i)).toString();
        QString address = info.left(info.indexOf(' '));
        QString name = info.right(info.length() - info.indexOf(' ') - 1);
        qDebug() << address << name;
        deviceTable->setItem(i, 0, new QTableWidgetItem(name));
        deviceTable->setItem(i, 1, new QTableWidgetItem(address));
        QTableWidgetItem* typeItem = new QTableWidgetItem();

        if(isBLE)
            typeItem->setText(tr("BLE"));
        else
            typeItem->setText(tr("RFCOMM"));
        typeItem->setData(Qt::UserRole, isBLE);
        deviceTable->setItem(i, 2, typeItem);
        m_shownDevices.append(address);
    }
}
#endif
