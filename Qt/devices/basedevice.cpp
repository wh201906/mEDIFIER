#include "basedevice.h"
#include "ui_basedevice.h"

#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#endif

BaseDevice::BaseDevice(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BaseDevice)
{
    ui->setupUi(this);

    ui->nameEdit->setMaxLength(m_maxNameLength);
    m_isSavingToFile = false;
#ifndef Q_OS_ANDROID
    ui->connectAudioButton->hide();
#endif

    connect(ui->noiseNormalButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInNoiseGroupClicked);
    connect(ui->noiseReductionButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInNoiseGroupClicked);
    connect(ui->noiseAmbientSoundButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInNoiseGroupClicked);

    connect(ui->SENormalButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInSoundEffectGroupClicked);
    connect(ui->SEPopButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInSoundEffectGroupClicked);
    connect(ui->SEClassicalButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInSoundEffectGroupClicked);
    connect(ui->SERockButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInSoundEffectGroupClicked);

    connect(ui->CSNormalBox, &QCheckBox::clicked, this, &BaseDevice::onCheckBoxInControlSettingsGroupClicked);
    connect(ui->CSNoiseReductionBox, &QCheckBox::clicked, this, &BaseDevice::onCheckBoxInControlSettingsGroupClicked);
    connect(ui->CSAmbientSoundBox, &QCheckBox::clicked, this, &BaseDevice::onCheckBoxInControlSettingsGroupClicked);

    connect(ui->LDACOFFButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInLDACGroupClicked);
    connect(ui->LDAC48kButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInLDACGroupClicked);
    connect(ui->LDAC96kButton, &QRadioButton::clicked, this, &BaseDevice::onBtnInLDACGroupClicked);

    connect(this, QOverload<const QByteArray&, const QString&, int>::of(&BaseDevice::pushCommand), this, QOverload<const QByteArray&, const QString&, int>::of(&BaseDevice::onCommandPushed));
    connect(this, QOverload<const char*, const QString&, int>::of(&BaseDevice::pushCommand), this, QOverload<const char*, const QString&, int>::of(&BaseDevice::onCommandPushed));
}

BaseDevice::~BaseDevice()
{
    delete ui;
}

void BaseDevice::setDeviceName(const QString &deviceName)
{
    m_deviceName = deviceName;
}

void BaseDevice::onBtnInNoiseGroupClicked()
{
    // This affects Ambient Sound
    if(ui->noiseNormalButton->isChecked())
        emit pushCommand("C101", "Noise Reduction", 1);
    else if(ui->noiseReductionButton->isChecked())
        emit pushCommand("C102", "Noise Reduction", 1);
    else if(ui->noiseAmbientSoundButton->isChecked())
        emit pushCommand("C103", "Noise Reduction", 1);
}

void BaseDevice::onBtnInSoundEffectGroupClicked()
{
    if(ui->SENormalButton->isChecked())
        emit pushCommand("C400", "Sound Effect");
    else if(ui->SEPopButton->isChecked())
        emit pushCommand("C401", "Sound Effect");
    else if(ui->SEClassicalButton->isChecked())
        emit pushCommand("C402", "Sound Effect");
    else if(ui->SERockButton->isChecked())
        emit pushCommand("C403", "Sound Effect");
}

void BaseDevice::onCheckBoxInControlSettingsGroupClicked()
{
    QByteArray cmd = "\xF1\x0A";
    char val = 0;
    if(ui->CSNormalBox->isChecked())
        val += 1;
    if(ui->CSNoiseReductionBox->isChecked())
        val += 2;
    if(ui->CSAmbientSoundBox->isChecked())
        val += 4;
    cmd += val;
    emit pushCommand(cmd, "Control Settings");
}

void BaseDevice::onBtnInLDACGroupClicked()
{
    // This triggers re-pairing, so it has the lowest priority
    if(ui->LDACOFFButton->isChecked())
        emit pushCommand("4900", "LDAC", 2);
    else if(ui->LDAC48kButton->isChecked())
        emit pushCommand("4901", "LDAC", 2);
    else if(ui->LDAC96kButton->isChecked())
        emit pushCommand("4902", "LDAC", 2);
}

void BaseDevice::on_gameModeBox_clicked()
{
    if(ui->gameModeBox->isChecked())
        emit pushCommand("0901", "Game Mode");
    else
        emit pushCommand("0900", "Game Mode");
}

void BaseDevice::on_ASSlider_valueChanged(int value)
{
    ui->ASBox->blockSignals(true);
    ui->ASBox->setValue(value);
    ui->ASBox->blockSignals(false);
}

void BaseDevice::on_ASBox_valueChanged(int arg1)
{
    ui->ASSlider->blockSignals(true);
    ui->ASSlider->setValue(arg1);
    ui->ASSlider->blockSignals(false);
}

void BaseDevice::on_ASSetButton_clicked()
{
    QByteArray cmd = "\xC1\x03";
    cmd += (char)(6 + ui->ASBox->value());
    emit pushCommand(cmd, "Ambient Sound");
    // setting ambient sound volume triggers ambient sound mode
    ui->noiseAmbientSoundButton->setChecked(true);
}

void BaseDevice::on_PVSlider_valueChanged(int value)
{
    ui->PVBox->blockSignals(true);
    ui->PVBox->setValue(value);
    ui->PVBox->blockSignals(false);
}

void BaseDevice::on_PVBox_valueChanged(int arg1)
{
    ui->PVSlider->blockSignals(true);
    ui->PVSlider->setValue(arg1);
    ui->PVSlider->blockSignals(false);
}

void BaseDevice::on_PVSetButton_clicked()
{
    QByteArray cmd = "\x06";
    cmd += (char)(ui->PVBox->value());
    emit pushCommand(cmd, "Prompt Volume");
}

void BaseDevice::on_shutdownTimerGroup_clicked()
{
    if(!ui->shutdownTimerGroup->isChecked())
        emit pushCommand("D2", "Shutdown Timer Enabled", 1);
}

void BaseDevice::on_STSlider_valueChanged(int value)
{
    ui->STBox->blockSignals(true);
    ui->STBox->setValue(value);
    ui->STBox->blockSignals(false);
}

void BaseDevice::on_STBox_valueChanged(int arg1)
{
    ui->STSlider->blockSignals(true);
    ui->STSlider->setValue(arg1);
    ui->STSlider->blockSignals(false);
}

void BaseDevice::on_STSetButton_clicked()
{
    // Warning:
    // This contains '\0', so the length must be specified
    QByteArray cmd = QByteArray("\xD1\x00", 2);
    cmd += (char)(ui->STBox->value());
    emit pushCommand(cmd, "Shutdown Timer");
}

void BaseDevice::on_poweroffButton_clicked()
{
    if(QMessageBox::question(this, tr("Info"), tr("The device will be powered off\nContinue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)
        emit sendCommand("CE");
}

void BaseDevice::on_disconenctButton_clicked()
{
    if(QMessageBox::question(this, tr("Info"), tr("The device will be disconnected\nContinue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)
        emit sendCommand("CD");
}

void BaseDevice::on_re_pairButton_clicked()
{
    if(QMessageBox::question(this, tr("Info"), tr("The device will get into pairing state\nContinue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)
        emit sendCommand("CF");
}

void BaseDevice::on_resetButton_clicked()
{
    if(QMessageBox::warning(this, tr("Info"), tr("The device will be reseted\nContinue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)
        emit sendCommand("07");
}

void BaseDevice::on_nameSetButton_clicked()
{
    QString name = ui->nameEdit->text();
    if(name.isEmpty())
        return;
    QByteArray nameBytes = name.toUtf8();
    // max length can be 24, 29, 30 or 35
    // the default length is 24(defined in basedevice.h)
    if(nameBytes.length() > m_maxNameLength)
    {
        emit showMessage(tr("The name is too long"));
        return;
    }
    QByteArray cmd = "\xCA";
    cmd += name.toUtf8();
    emit pushCommand(cmd, "Name");
}

void BaseDevice::processData(const QByteArray& data)
{
    const char head = data[0];
    const int len = (int)data[1];
    if(head == '\xBB')
    {
        // cmd + single byte response
        if(len == 2)
        {
            const char cmd = data[2];
            const char ch = data[3];
            if(cmd == '\xD5')
            {
                ui->SENormalButton->setChecked(ch == '\x00');
                ui->SEPopButton->setChecked(ch == '\x01');
                ui->SEClassicalButton->setChecked(ch == '\x02');
                ui->SERockButton->setChecked(ch == '\x03');
            }
            else if(cmd == '\x08')
            {
                ui->gameModeBox->setChecked(ch == '\x01');
            }
            else if(cmd == '\xD0')
            {
                ui->batteryLabel->setText(QString::number((int)ch) + "%");
            }
            else if(cmd == '\x48')
            {
                ui->LDACOFFButton->setChecked(ch == '\x00');
                ui->LDAC48kButton->setChecked(ch == '\x01');
                ui->LDAC96kButton->setChecked(ch == '\x02');
            }
            else if(cmd == '\x05')
            {
                ui->PVBox->setValue((int)ch);
            }
            else if(cmd == '\xD3')
            {
                ui->shutdownTimerGroup->setChecked(ch != '\x00');
            }
            else if(cmd == '\xD7')
            {
                ui->autoPoweroffBox->setChecked(ch == '\x01');
            }
        }
        else if(len > 2)
        {
            const char cmd = data[2];
            if(cmd == '\xC8' && len == 7)
            {
                QString address = data.right(6).toHex(':');
                ui->MACLabel->setText(address);
                m_address = address;
            }
            else if(cmd == '\xC6' && len == 4)
            {
                ui->firmwareLabel->setText(data.right(3).toHex('.'));
            }
            else if(cmd == '\xCC' && len == 3)
            {
                char mode = data[3];
                int ASVolume = (int)data[4] - 6;
                ui->noiseNormalButton->setChecked(mode == '\x01');
                ui->noiseReductionButton->setChecked(mode == '\x02');
                ui->noiseAmbientSoundButton->setChecked(mode == '\x03');
                ui->ASBox->setValue(ASVolume);
            }
            else if(cmd == '\xC9')
            {
                QString deviceName = QString::fromUtf8(data.mid(3));
                ui->nameEdit->setText(deviceName);
            }
            else if(cmd == '\xF0' && len == 3 && data[3] == '\x0A')
            {
                quint8 mode = data[4];
                ui->CSNormalBox->setChecked(mode & 1u);
                ui->CSNoiseReductionBox->setChecked(mode & 2u);
                ui->CSAmbientSoundBox->setChecked(mode & 4u);
            }
            else if(cmd == '\xD3' && len == 3)
            {
                int shutdownTimeout = data[4];
                ui->shutdownTimerGroup->setChecked(true);
                ui->STBox->setValue(shutdownTimeout);
            }
        }
    }
    else if(head == '\xCC')
    {

    }
}

void BaseDevice::readSettings()
{
    const int interval = 150;
    int i = 0;

    QTimer::singleShot(i, [ = ] {on_batteryGetButton_clicked();});
    i += interval;
    QTimer::singleShot(i, [ = ] {on_MACGetButton_clicked();});
    i += interval;
    QTimer::singleShot(i, [ = ] {on_firmwareGetButton_clicked();});
    i += interval;
    if(ui->ambientSoundGroup->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("CC");});
        i += interval;
    }
    if(ui->nameGroup->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("C9");});
        i += interval;
    }
    if(ui->soundEffectGroup->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("D5");});
        i += interval;
    }
    if(ui->gameModeBox->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("08");});
        i += interval;
    }
    if(ui->controlSettingsGroup->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("F00A");});
        i += interval;
    }
    if(ui->LDACGroup->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("48");});
        i += interval;
    }
    if(ui->promptVolumeGroup->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("05");});
        i += interval;
    }
    if(ui->shutdownTimerGroup->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("D3");});
        i += interval;
    }
    if(ui->autoPoweroffBox->isVisible())
    {
        QTimer::singleShot(i, [ = ] {emit sendCommand("D7");});
        i += interval;
    }
}

void BaseDevice::on_batteryGetButton_clicked()
{
    emit sendCommand("D0");
}

void BaseDevice::on_MACGetButton_clicked()
{
    emit sendCommand("C8");
}

void BaseDevice::on_firmwareGetButton_clicked()
{
    emit sendCommand("C6");
}

void BaseDevice::on_cmdSentButton_clicked()
{
    emit sendCommand(QByteArray::fromHex(ui->cmdEdit->text().toLatin1()), ui->cmdRawBox->isChecked());
}

void BaseDevice::on_PCPlayButton_clicked()
{
    emit sendCommand("C200");
}

void BaseDevice::on_PCPauseButton_clicked()
{
    emit sendCommand("C201");
}

void BaseDevice::on_PCVolUpButton_clicked()
{
    emit sendCommand("C202");
}

void BaseDevice::on_PCVolDownButton_clicked()
{
    emit sendCommand("C203");
}

void BaseDevice::on_PCPrevButton_clicked()
{
    emit sendCommand("C205");
}

void BaseDevice::on_PCNextButton_clicked()
{
    emit sendCommand("C204");
}

void BaseDevice::on_autoPoweroffBox_clicked()
{
    if(ui->autoPoweroffBox->isChecked())
        emit pushCommand("D601", "Auto Poweroff");
    else
        emit pushCommand("D600", "Auto Poweroff");
}

bool BaseDevice::setMaxNameLength(int length)
{
    if(length <= 0)
        return false;
    m_maxNameLength = length;
    return true;
}

bool BaseDevice::hideWidget(const QString& widgetName)
{
    if(widgetName.isEmpty())
        return false;
    QWidget* widget = findChild<QWidget *>(widgetName);
    if(widget == nullptr)
        return false;
    widget->hide();
    return true;
}

void BaseDevice::clearAddress()
{
    m_address.clear();
}

void BaseDevice::onCommandPushed(const QByteArray &cmd, const QString &name, int priority)
{
    if(m_isSavingToFile)
    {
        if(m_cmdInFile == nullptr)
        {
            qDebug() << "Warning: m_cmdInFile not initialized";
            return;
        }
        QJsonObject cmdObject;
        cmdObject.insert("cmd", QString::fromLatin1(cmd.toHex()));
        if(!name.isEmpty())
            cmdObject.insert("name", name);
        if(priority > 0)
            cmdObject.insert("priority", priority);
        m_cmdInFile->append(cmdObject);
    }
    else
        emit sendCommand(cmd);
}

void BaseDevice::onCommandPushed(const char* hexCmd, const QString& name, int priority)
{
    onCommandPushed(QByteArray::fromHex(hexCmd), name, priority);
}

void BaseDevice::on_fileSaveButton_clicked()
{
    QString caption = tr("Save Settings");
#ifdef Q_OS_ANDROID
    // On Android, caption will be used as the default filename
    caption = m_deviceName + ".json";
#endif
    QString filename = QFileDialog::getSaveFileName(this, caption, m_deviceName + ".json");
    if(filename.isEmpty())
        return;

    m_isSavingToFile = true;
    m_cmdInFile = new QJsonArray;

    if(ui->soundEffectGroup->isVisible())
        onBtnInSoundEffectGroupClicked();
    if(ui->controlSettingsGroup->isVisible())
        onCheckBoxInControlSettingsGroupClicked();
    if(ui->LDACGroup->isVisible())
        onBtnInLDACGroupClicked();
    if(ui->gameModeBox->isVisible())
        on_gameModeBox_clicked();
    if(ui->ambientSoundGroup->isVisible())
    {
        onBtnInNoiseGroupClicked();
        on_ASSetButton_clicked();
    }
    if(ui->promptVolumeGroup->isVisible())
        on_PVSetButton_clicked();
    if(ui->shutdownTimerGroup->isVisible())
    {
        on_shutdownTimerGroup_clicked();
        on_STSetButton_clicked();
    }
    if(ui->nameGroup->isVisible())
        on_nameSetButton_clicked();
    if(ui->autoPoweroffBox->isVisible())
        on_autoPoweroffBox_clicked();

    QJsonObject settingsObj;
    settingsObj.insert("name", m_deviceName);
    settingsObj.insert("commands", *m_cmdInFile);
    qDebug() << settingsObj;
    QFile settingsFile(filename);
    // QFile::Truncate is necessary there
    if(!settingsFile.open(QFile::WriteOnly | QFile::Truncate))
        QMessageBox::information(this, tr("Error"), tr("Failed to save to") + "\n" + filename);
    else
    {
        settingsFile.write(QJsonDocument(settingsObj).toJson());
        QMessageBox::information(this, tr("Info"), tr("Saved"));
    }

    delete(m_cmdInFile);
    m_cmdInFile = nullptr;
    m_isSavingToFile = false;
}


void BaseDevice::on_fileWriteDeviceButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this);
    if(filename.isEmpty())
        return;

    QFile settingsFile(filename);
    if(!settingsFile.open(QFile::ReadOnly))
    {
        QMessageBox::information(this, tr("Error"), tr("Failed to open") + "\n" + filename);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(settingsFile.readAll());
    if(doc.isNull())
    {
        QMessageBox::information(this, tr("Error"), tr("Invalid JSON file"));
        return;
    }
    QJsonObject settingsObj = doc.object();
    if(settingsObj.isEmpty() || !settingsObj.contains("commands") || !settingsObj["commands"].isArray())
    {
        QMessageBox::information(this, tr("Error"), tr("Invalid format"));
        return;
    }
    const QJsonArray cmdInFile = settingsObj["commands"].toArray();

    const int interval = 150;
    int i = 0;
    QList<QJsonValue> cmdList[3];
    for(const auto& cmdItem : cmdInFile)
    {
        const QString cmd = cmdItem["cmd"].toString();
        int priority = cmdItem["priority"].toInt(0);
        if(cmd.isEmpty())
            continue;
        else if(priority < 3)
            cmdList[priority].append(cmdItem);
    }
    for(int priority = 0; priority < 3; priority++)
    {
        for(const auto& cmdItem : qAsConst(cmdList[priority]))
        {
            const QString cmd = cmdItem["cmd"].toString();
            const QString name = cmdItem["name"].toString();
            QTimer::singleShot(i, [ = ] {emit sendCommand(QByteArray::fromHex(cmd.toLatin1()));});
            i += interval;
        }
    }
    QTimer::singleShot(i, [ = ] {QMessageBox::information(this, tr("Info"), tr("Done"));});

}

void BaseDevice::on_connectAudioButton_clicked()
{
#ifdef Q_OS_ANDROID
    // Get MAC address for audio
    on_MACGetButton_clicked();

    // Disconnect
    QTimer::singleShot(300, [ = ]
    {
        if(m_address.isEmpty())
            qDebug() << "Error: m_address is not set";
        else
            emit sendCommand("CD"); // on_disconenctButton_clicked() without confirmation
    });

    // Connect Audio
    QTimer::singleShot(600, [ = ]
    {
        if(m_address.isEmpty())
        {
            qDebug() << "Error: m_address is not set";
            return;
        }
        QAndroidJniEnvironment androidEnv;
        QAndroidJniObject addressObj = QAndroidJniObject::fromString(m_address);
        QtAndroid::androidActivity().callMethod<void>("connectToDevice", "(Ljava/lang/String;)V", addressObj.object<jstring>());
    });
#endif
}

