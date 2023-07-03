#ifndef DEVFORM_H
#define DEVFORM_H

#include <QWidget>

namespace Ui
{
class DevForm;
}

class DevForm : public QWidget
{
    Q_OBJECT

public:
    explicit DevForm(QWidget *parent = nullptr);
    ~DevForm();

public slots:
    void handleDevMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private slots:
    void on_copyLogButton_clicked();

    void on_verboseLogBox_clicked();

private:
    Ui::DevForm *ui;

    bool m_isLogVerbose = false;

signals:
    void showMessage(const QString& msg);
};

#endif // DEVFORM_H
