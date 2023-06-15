#include "deviceform.h"
#include "ui_deviceform.h"

#include <QDebug>
#include <QBluetoothUuid>
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
    ui->deviceTypeBox->addItem(tr("RFCOMM"), false);
    ui->deviceTypeBox->addItem(tr("BLE"), true);

    ui->disconnectButton->setVisible(false);
    ui->searchStopButton->setVisible(false);

    ui->deviceTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->searchRFCOMMButton, &QPushButton::clicked, this, &DeviceForm::onSearchButtonClicked);
    connect(ui->searchBLEButton, &QPushButton::clicked, this, &DeviceForm::onSearchButtonClicked);
    connect(ui->deviceTableWidget, &QTableWidget::cellClicked, this, &DeviceForm::onDeviceTableCellClicked);

    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceForm::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceForm::onDiscoverFinished);
}

DeviceForm::~DeviceForm()
{
    delete ui;
}

void DeviceForm::onSearchButtonClicked()
{
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
    m_discoveryAgent->start(m_isCurrDiscoveryMethodBLE ?
                            QBluetoothDeviceDiscoveryAgent::LowEnergyMethod :
                            QBluetoothDeviceDiscoveryAgent::ClassicMethod);
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

void DeviceForm::on_connectButton_clicked()
{
    bool isBLE = ui->deviceTypeBox->currentData().toBool();
    emit connectTo(ui->deviceAddressEdit->text(), isBLE);
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

#ifdef Q_OS_ANDROID
void DeviceForm::getBondedTarget(bool isBLE)
{
    QAndroidJniEnvironment env;
    QtAndroid::PermissionResult r = QtAndroid::checkPermission("android.permission.ACCESS_FINE_LOCATION");
    if(r == QtAndroid::PermissionResult::Denied)
    {
        QtAndroid::requestPermissionsSync(QStringList() << "android.permission.ACCESS_FINE_LOCATION");
        r = QtAndroid::checkPermission("android.permission.ACCESS_FINE_LOCATION");
        if(r == QtAndroid::PermissionResult::Denied)
        {
            qDebug() << "failed to request";
        }
    }
    qDebug() << "has permission";
    QAndroidJniObject array = QtAndroid::androidActivity().callObjectMethod("getBondedDevices", "(Z)[Ljava/lang/String;", isBLE);
    int arraylen = env->GetArrayLength(array.object<jarray>());
    qDebug() << "arraylen:" << arraylen;
    QTableWidget* deviceTable = ui->deviceTableWidget;
    deviceTable->setRowCount(arraylen);
    for(int i = 0; i < arraylen; i++)
    {
        QString info = QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(array.object<jobjectArray>(), i)).toString();
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

