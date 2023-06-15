#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "deviceform.h"
#include "comm.h"
#include "w820nbplusform.h"


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showMessage(const QString &msg);
private:
    Ui::MainWindow *ui;

    DeviceForm* m_deviceForm = nullptr;
    Comm* m_comm = nullptr;
    bool m_connected = false;
    W820NBPlusForm* m_w820p = nullptr;
private slots:
    void connectToDevice(const QString &address, bool isBLE);
    void disconnectDevice();
    void onCommStateChanged(bool state);
    void on_readSettingsButton_clicked();

signals:
    void commStateChanged(bool connected);
    void readSettings();
};
#endif // MAINWINDOW_H
