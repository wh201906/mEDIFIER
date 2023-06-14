#include "deviceform.h"
#include "ui_deviceform.h"

#include <QDebug>
#include <QBluetoothUuid>

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
    QTableWidget* deviceList = ui->deviceTableWidget;
    int i;

    i = deviceList->rowCount();
    deviceList->setRowCount(i + 1);
    deviceList->setItem(i, 0, new QTableWidgetItem(name));
    deviceList->setItem(i, 1, new QTableWidgetItem(address));
    QTableWidgetItem* typeItem = new QTableWidgetItem();

    if(m_isCurrDiscoveryMethodBLE)
        typeItem->setText(tr("BLE"));
    else
        typeItem->setText(tr("RFCOMM"));
    typeItem->setData(Qt::UserRole, m_isCurrDiscoveryMethodBLE);
    deviceList->setItem(i, 2, typeItem);


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

