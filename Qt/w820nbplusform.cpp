#include "w820nbplusform.h"
#include "ui_w820nbplusform.h"

#include <QDebug>

W820NBPlusForm::W820NBPlusForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::W820NBPlusForm)
{
    ui->setupUi(this);

    connect(ui->noiseNormalButton, &QRadioButton::clicked, this, &W820NBPlusForm::onBtnInNoiseGroupClicked);
    connect(ui->noiseReductionButton, &QRadioButton::clicked, this, &W820NBPlusForm::onBtnInNoiseGroupClicked);
    connect(ui->noiseAmbientSoundButton, &QRadioButton::clicked, this, &W820NBPlusForm::onBtnInNoiseGroupClicked);

    connect(ui->SENormalButton, &QRadioButton::clicked, this, &W820NBPlusForm::onBtnInSoundEffectGroupClicked);
    connect(ui->SEPopButton, &QRadioButton::clicked, this, &W820NBPlusForm::onBtnInSoundEffectGroupClicked);
    connect(ui->SEClassicalButton, &QRadioButton::clicked, this, &W820NBPlusForm::onBtnInSoundEffectGroupClicked);
    connect(ui->SERockButton, &QRadioButton::clicked, this, &W820NBPlusForm::onBtnInSoundEffectGroupClicked);

    connect(ui->CSNormalBox, &QCheckBox::clicked, this, &W820NBPlusForm::onCheckBoxInControlSettingsGroupClicked);
    connect(ui->CSNoiseReductionBox, &QCheckBox::clicked, this, &W820NBPlusForm::onCheckBoxInControlSettingsGroupClicked);
    connect(ui->CSAmbientSoundBox, &QCheckBox::clicked, this, &W820NBPlusForm::onCheckBoxInControlSettingsGroupClicked);
}

W820NBPlusForm::~W820NBPlusForm()
{
    delete ui;
}

void W820NBPlusForm::onBtnInNoiseGroupClicked()
{
    if(ui->noiseNormalButton->isChecked())
        emit sendCommand("C101");
    else if(ui->noiseReductionButton->isChecked())
        emit sendCommand("C102");
    else if(ui->noiseAmbientSoundButton->isChecked())
        emit sendCommand("C103");
}



void W820NBPlusForm::onBtnInSoundEffectGroupClicked()
{
    if(ui->SENormalButton->isChecked())
        emit sendCommand("C400");
    else if(ui->SEPopButton->isChecked())
        emit sendCommand("C401");
    else if(ui->SEClassicalButton->isChecked())
        emit sendCommand("C402");
    else if(ui->SERockButton->isChecked())
        emit sendCommand("C403");
}

void W820NBPlusForm::onCheckBoxInControlSettingsGroupClicked()
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
    qDebug() << cmd;
    emit sendCommand(cmd);
}

void W820NBPlusForm::onBtnInLDACGroupClicked()
{
    if(ui->LDACOFFButton->isChecked())
        emit sendCommand("C400");
    else if(ui->LDAC48kButton->isChecked())
        emit sendCommand("C401");
    else if(ui->LDAC96kButton->isChecked())
        emit sendCommand("C402");
}

void W820NBPlusForm::on_gameModeBox_clicked()
{
    if(ui->gameModeBox->isChecked())
        emit sendCommand("0901");
    else
        emit sendCommand("0900");
}


void W820NBPlusForm::on_ASSlider_valueChanged(int value)
{
    ui->ASBox->blockSignals(true);
    ui->ASBox->setValue(value);
    ui->ASBox->blockSignals(false);
}


void W820NBPlusForm::on_ASBox_valueChanged(int arg1)
{
    ui->ASSlider->blockSignals(true);
    ui->ASSlider->setValue(arg1);
    ui->ASSlider->blockSignals(false);
}


void W820NBPlusForm::on_ASSetButton_clicked()
{
    QByteArray cmd = "\xC1\x03";
    cmd += (char)(3 + ui->ASBox->value());
    emit sendCommand(cmd);
}


void W820NBPlusForm::on_PVSlider_valueChanged(int value)
{
    ui->PVBox->blockSignals(true);
    ui->PVBox->setValue(value);
    ui->PVBox->blockSignals(false);
}


void W820NBPlusForm::on_PVBox_valueChanged(int arg1)
{
    ui->PVSlider->blockSignals(true);
    ui->PVSlider->setValue(arg1);
    ui->PVSlider->blockSignals(false);
}


void W820NBPlusForm::on_PVSetButton_clicked()
{
    QByteArray cmd = "\x06";
    cmd += (char)(ui->PVBox->value());
    emit sendCommand(cmd);
}


void W820NBPlusForm::on_shutdownTimerGroup_clicked()
{
    if(!ui->shutdownTimerGroup->isChecked())
        emit sendCommand("D2");
}


void W820NBPlusForm::on_STSlider_valueChanged(int value)
{
    ui->STBox->blockSignals(true);
    ui->STBox->setValue(value);
    ui->STBox->blockSignals(false);
}


void W820NBPlusForm::on_STBox_valueChanged(int arg1)
{
    ui->STSlider->blockSignals(true);
    ui->STSlider->setValue(arg1);
    ui->STSlider->blockSignals(false);
}


void W820NBPlusForm::on_STSetButton_clicked()
{
    QByteArray cmd = "\xD1\x00";
    cmd += (char)(ui->STBox->value());
    emit sendCommand(cmd);
}


void W820NBPlusForm::on_poweroffButton_clicked()
{
    emit sendCommand("CE");
}


void W820NBPlusForm::on_disconenctButton_clicked()
{
    emit sendCommand("CD");
}


void W820NBPlusForm::on_re_pairButton_clicked()
{
    emit sendCommand("CF");
}


void W820NBPlusForm::on_resetButton_clicked()
{
    emit sendCommand("07");
}

