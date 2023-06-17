#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commrfcomm.h"
#include "commble.h"
#include "w820nbplus.h"

#include <QDebug>
#include <QScroller>
#include <QScrollBar>
#include <QMessageBox>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    m_deviceForm = new DeviceForm;
    m_w820p = new W820NBPlus;
    ui->tabWidget->setTabText(0, m_w820p->windowTitle());
    ui->tabWidget->insertTab(0, m_deviceForm, tr("Device"));
    ui->scrollAreaWidgetContents->layout()->addWidget(m_w820p);
    ui->tabWidget->setCurrentIndex(0);

    connect(m_deviceForm, &DeviceForm::connectTo, this, &MainWindow::connectToDevice);
    connect(m_deviceForm, &DeviceForm::disconnectDevice, this, &MainWindow::disconnectDevice);
    connect(this, &MainWindow::commStateChanged, m_deviceForm, &DeviceForm::onCommStateChanged);

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

void MainWindow::connectToDevice(const QString& address, bool isBLE)
{
    if(m_comm != nullptr)
    {
        m_comm->deleteLater();
        m_comm = nullptr;
    }
    if(isBLE)
    {
        m_comm = new CommBLE;
    }
    else
        m_comm = new CommRFCOMM;

    connect(m_comm, &Comm::stateChanged, this, &MainWindow::onCommStateChanged);
    connect(m_w820p, QOverload<const QByteArray&, bool>::of(&BaseDevice::sendCommand), m_comm, QOverload<const QByteArray&, bool>::of(&Comm::sendCommand));
    connect(m_w820p, QOverload<const char*, bool>::of(&BaseDevice::sendCommand), m_comm, QOverload<const char*, bool>::of(&Comm::sendCommand));
    connect(m_comm, &Comm::newData, m_w820p, &BaseDevice::processData);
    connect(this, &MainWindow::readSettings, m_w820p, &BaseDevice::readSettings);
    connect(m_comm, &Comm::showMessage, this, &MainWindow::showMessage);
    connect(m_w820p, &BaseDevice::showMessage, this, &MainWindow::showMessage);
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
    emit readSettings();
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
