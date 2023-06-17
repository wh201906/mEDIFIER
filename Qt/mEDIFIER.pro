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
    commble.cpp \
    main.cpp \
    mainwindow.cpp \
    comm.cpp \
    commrfcomm.cpp \
    deviceform.cpp \
    w820nbplusform.cpp \
    winbthelper.cpp

HEADERS += \
    commble.h \
    mainwindow.h \
    comm.h \
    commrfcomm.h \
    deviceform.h \
    w820nbplusform.h \
    winbthelper.h

FORMS += \
    mainwindow.ui \
    deviceform.ui \
    w820nbplusform.ui

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
VERSION = 0.0.3
# Expose VERSION to the source files.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# For bluetoothapis.h
win32: LIBS += -lBthprops
