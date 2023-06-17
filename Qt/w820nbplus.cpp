#include "w820nbplus.h"
#include "ui_basedevice.h"

W820NBPlus::W820NBPlus()
{
    setWindowTitle(tr("W820NB Plus"));
    // the max length of name for W820NB Plus is 30
    m_maxNameLength = 30;
}
