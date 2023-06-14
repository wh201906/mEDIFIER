#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commrfcomm.h"

#include <QDebug>
#include <QScroller>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    m_deviceForm = new DeviceForm;
    m_w820p = new W820NBPlusForm;
    ui->tabWidget->insertTab(0, m_deviceForm, tr("Device"));
    ui->scrollAreaWidgetContents->layout()->addWidget(m_w820p);

    connect(m_deviceForm, &DeviceForm::connectTo, this, &MainWindow::connectToDevice);
    connect(m_deviceForm, &DeviceForm::disconnectDevice, this, &MainWindow::disconnectDevice);
    connect(this, &MainWindow::commStateChanged, m_deviceForm, &DeviceForm::onCommStateChanged);


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
        ;
//        m_comm = new CommBLE;
    }
    else
        m_comm = new CommRFCOMM;

    connect(m_comm, &Comm::stateChanged, this, &MainWindow::onCommStateChanged);
    connect(m_w820p, QOverload<const QByteArray&, bool>::of(&W820NBPlusForm::sendCommand), m_comm, QOverload<const QByteArray&, bool>::of(&Comm::sendCommand));
    connect(m_w820p, QOverload<const char*, bool>::of(&W820NBPlusForm::sendCommand), m_comm, QOverload<const char*, bool>::of(&Comm::sendCommand));
    connect(m_comm, &Comm::newData, m_w820p, &W820NBPlusForm::processData);
    connect(this, &MainWindow::readSettings, m_w820p, &W820NBPlusForm::readSettings);
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

