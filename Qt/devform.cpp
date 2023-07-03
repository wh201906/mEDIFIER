#include "devform.h"
#include "ui_devform.h"

#include <QDateTime>

DevForm::DevForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DevForm)
{
    ui->setupUi(this);
    connect(ui->clearLogButton, &QPushButton::clicked, ui->logEdit, &QPlainTextEdit::clear);
}

DevForm::~DevForm()
{
    delete ui;
}

void DevForm::on_copyLogButton_clicked()
{
    QTextCursor cursor = ui->logEdit->textCursor();
    ui->logEdit->selectAll();
    ui->logEdit->copy();
    ui->logEdit->setTextCursor(cursor);
    emit showMessage(tr("Copied"));
}

void DevForm::handleDevMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString file = context.file ? context.file : "";
    QString function = context.function ? context.function : "";
    QString extraInfo;
    if(file.isEmpty() || function.isEmpty())
        extraInfo = QString(" (%1:%2, %3)").arg(file).arg(context.line).arg(function);

    QString typeStr;
    switch(type)
    {
    case QtDebugMsg:
        typeStr = "Debug";
        break;
    case QtInfoMsg:
        typeStr = "Info";
        break;
    case QtWarningMsg:
        typeStr = "Warning";
        break;
    case QtCriticalMsg:
        typeStr = "Critical";
        break;
    case QtFatalMsg:
        typeStr = "Fatal";
        break;
    }

    if(m_isLogVerbose)
        ui->logEdit->appendPlainText(QString("[%1]%2: %3%4").arg(QDateTime::currentDateTime().toString(Qt::ISODate), typeStr, msg, extraInfo));
    else
        ui->logEdit->appendPlainText(QString("%1%2").arg(msg, extraInfo));
}

void DevForm::on_verboseLogBox_clicked()
{
    m_isLogVerbose = ui->verboseLogBox->isChecked();
}

