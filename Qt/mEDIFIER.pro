QT += core gui bluetooth
android {
    QT += androidextras
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    devform.cpp \
    main.cpp \
    mainwindow.cpp \
    comms/comm.cpp \
    comms/commrfcomm.cpp \
    comms/commble.cpp \
    comms/winbthelper.cpp \
    deviceform.cpp \
    devices/basedevice.cpp

HEADERS += \
    devform.h \
    mainwindow.h \
    comms/comm.h \
    comms/commrfcomm.h \
    comms/commble.h \
    comms/winbthelper.h \
    deviceform.h \
    devices/basedevice.h

FORMS += \
    devform.ui \
    mainwindow.ui \
    deviceform.ui \
    devices/basedevice.ui

TRANSLATIONS += \
    mEDIFIER_zh_CN.ts

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
DISTFILES += \
    android/AndroidManifest.xml

# Remember to change version in AndroidManifest.xml
VERSION = 0.1.0
# Expose VERSION to the source files.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
QMAKE_TARGET_PRODUCT = "mEDIFIER"
QMAKE_TARGET_DESCRIPTION = "Alternative to Edifier Connect"
QMAKE_TARGET_COMPANY = "wh201906"

# For bluetoothapis.h on Windows, with MinGW
win32: LIBS += -lBthprops

RESOURCES += \
    devices/devices.qrc
