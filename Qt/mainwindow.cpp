#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "comms/commrfcomm.h"
#include "comms/commble.h"

#include <QDebug>
#include <QScroller>
#include <QScrollBar>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    m_deviceForm = new DeviceForm;
    m_device = new BaseDevice;
    m_device->hideWidget("");
    ui->tabWidget->setTabText(0, m_device->windowTitle());
    ui->tabWidget->insertTab(0, m_deviceForm, tr("Device"));
    ui->scrollAreaWidgetContents->layout()->addWidget(m_device);
    ui->tabWidget->setCurrentIndex(0);

    connect(m_deviceForm, &DeviceForm::connectTo, this, &MainWindow::connectToDevice);
    connect(m_deviceForm, &DeviceForm::disconnectDevice, this, &MainWindow::disconnectDevice);
    connect(m_deviceForm, &DeviceForm::showMessage, this, &MainWindow::showMessage);
    connect(this, &MainWindow::commStateChanged, m_deviceForm, &DeviceForm::onCommStateChanged);

    loadDeviceInfo();

#ifdef Q_OS_ANDROID
    ui->statusBar->hide();
#endif
    QScroller::grabGesture(ui->scrollArea);
//    ui->scrollArea->horizontalScrollBar()->setEnabled(false);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadDeviceInfo()
{
    QFile deviceInfoFile(":/devices/deviceinfo.json");
    deviceInfoFile.open(QIODevice::ReadOnly);
    QJsonDocument deviceInfoDoc = QJsonDocument::fromJson(deviceInfoFile.readAll());
    deviceInfoFile.close();
    if(deviceInfoDoc.isObject())
        m_deviceInfo = new QJsonObject(deviceInfoDoc.object());
    else
        qDebug() << "Failed to load Device Info";

    for(auto it = m_deviceInfo->constBegin(); it != m_deviceInfo->constEnd(); ++it)
    {
        QJsonObject details = it->toObject();
        ui->deviceBox->addItem(tr(details["Name"].toString().toUtf8()), it.key());
        QString serviceUUID = details["UniqueServiceUUID"].toString();
        if(!serviceUUID.isEmpty())
            m_deviceServiceMap[QBluetoothUuid(serviceUUID)] = it.key();
    }
}

void MainWindow::connectToDevice(const QString& address, bool isBLE)
{
    if(m_comm != nullptr)
    {
        m_comm->deleteLater();
        m_comm = nullptr;
    }
    if(isBLE)
        m_comm = new CommBLE;
    else
        m_comm = new CommRFCOMM;

    connect(m_comm, &Comm::stateChanged, this, &MainWindow::onCommStateChanged);
    connect(m_comm, &Comm::showMessage, this, &MainWindow::showMessage);
    connectDevice2Comm();

    m_comm->open(address);
}

void MainWindow::disconnectDevice()
{
    m_comm->close();
    m_connected = false;
    emit commStateChanged(false);
}

void MainWindow::onCommStateChanged(bool state)
{
    if(!m_connected && state)
    {
        m_connected = true;
        emit commStateChanged(true);
    }
    else if(m_connected && !state)
    {
        m_connected = false;
        emit commStateChanged(false);
    }
}

void MainWindow::on_readSettingsButton_clicked()
{
    if(m_connected)
        emit readSettings();
    else
        showMessage(tr("Device not connected"));
}

void MainWindow::showMessage(const QString& msg)
{
#ifdef Q_OS_ANDROID
    QtAndroid::runOnAndroidThread([ = ]
    {
        QAndroidJniObject javaString = QAndroidJniObject::fromString(msg);
        QAndroidJniObject toast = QAndroidJniObject::callStaticObjectMethod("android/widget/Toast", "makeText",
                "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;",
                QtAndroid::androidActivity().object(),
                javaString.object(),
                jint(0)); // short toast
        toast.callMethod<void>("show");
    });
#else
    ui->statusBar->showMessage(msg, 2000); // 2000ms is the duration of short toast
#endif
}

void MainWindow::changeDevice(const QString& deviceName)
{
    // the deviceName is the key in deviceinfo.json, not "Name"
    if(!m_deviceInfo->contains(deviceName))
        return;
    if(m_device != nullptr)
        m_device->deleteLater();
    QJsonObject details = m_deviceInfo->value(deviceName).toObject();
    m_device = new BaseDevice;
    m_device->setWindowTitle(tr(details["Name"].toString().toUtf8()));
    m_device->setMaxNameLength(details["MaxNameLength"].toInt());
    const QVariantList hiddenFeatureList = details["HiddenFeatures"].toArray().toVariantList();
    for(const auto& it : hiddenFeatureList)
    {
        m_device->hideWidget(it.toString());
    }
    connectDevice2Comm();

    ui->tabWidget->setTabText(1, m_device->windowTitle());
    ui->scrollAreaWidgetContents->layout()->addWidget(m_device);
}

void MainWindow::on_deviceBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    changeDevice(ui->deviceBox->currentData().toString());
}

void MainWindow::connectDevice2Comm()
{
    if(m_device == nullptr || m_comm == nullptr)
    {
        qDebug() << "Error: Failed to connect Device to Comm";
        return;
    }
    connect(m_device, QOverload<const QByteArray&, bool>::of(&BaseDevice::sendCommand), m_comm, QOverload<const QByteArray&, bool>::of(&Comm::sendCommand));
    connect(m_device, QOverload<const char*, bool>::of(&BaseDevice::sendCommand), m_comm, QOverload<const char*, bool>::of(&Comm::sendCommand));
    connect(m_comm, &Comm::newData, m_device, &BaseDevice::processData);
    connect(this, &MainWindow::readSettings, m_device, &BaseDevice::readSettings);
    connect(m_device, &BaseDevice::showMessage, this, &MainWindow::showMessage);
    connect(m_comm, &Comm::deviceFeature, this, &MainWindow::processDeviceFeature);
}

void MainWindow::processDeviceFeature(const QString& feature, bool isBLE)
{
    if(isBLE)
    {
        QBluetoothUuid serviceUUID = QBluetoothUuid(feature);
        qDebug() << "Device service UUID:" << feature;
        if(m_deviceServiceMap.contains(serviceUUID))
        {
            int index = ui->deviceBox->findData(m_deviceServiceMap[serviceUUID]);
            ui->deviceBox->setCurrentIndex(index);
            showMessage(tr("Device detected") + ": " + ui->deviceBox->currentText());
        }
    }
}

const char* MainWindow::m_translatedNames[] =
{
    QT_TR_NOOP("Generic Device"),
    QT_TR_NOOP("W820NB Double Gold"),
    QT_TR_NOOP("W200BT Plus"),
};
