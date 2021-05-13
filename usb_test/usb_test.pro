QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FPGA_device.cpp \
    USB_device.cpp \
    ft600_drvinterface.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    FPGA_device.h \
    USB_device.h \
    defines.h \
    ft600_drvinterface.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

OS_SUFFIX =win64
EXT_LIB =c:/work/win/extlib

#--- FTDI library ---
    FTD3XX_PATH = $$EXT_LIB/ftdi/ftd3xx/1.3.0.4

    INCLUDEPATH += $${FTD3XX_PATH}/include/win
    LIBS += $${FTD3XX_PATH}/lib/$$OS_SUFFIX/Static/ftd3xx.lib
