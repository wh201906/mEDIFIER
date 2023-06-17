#include "w200btplus.h"
#include "ui_basedevice.h"

W200BTPlus::W200BTPlus()
{
    setWindowTitle(tr("W200BT Plus"));
    // the max length of name for W200BT Plus is 24
    m_maxNameLength = 24;
    ui->noiseGroup->hide();
    ui->ambientSoundGroup->hide();
    ui->controlSettingsGroup->hide();
    ui->LDACGroup->hide();
    ui->gameModeBox->hide();
}
