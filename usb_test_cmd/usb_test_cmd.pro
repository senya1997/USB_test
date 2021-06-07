TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        FPGA_device.cpp \
        USB_device.cpp \
        ft600_drvinterface.cpp \
        main.cpp

HEADERS += \
    FPGA_device.h \
    USB_device.h \
    defines.h \
    ft600_drvinterface.h

OS_SUFFIX =win64
EXT_LIB =c:/work/win/extlib

#--- FTDI library ---
    FTD3XX_PATH = $$EXT_LIB/ftdi/ftd3xx/1.3.0.4

    INCLUDEPATH += $${FTD3XX_PATH}/include/win
    LIBS += $${FTD3XX_PATH}/lib/$$OS_SUFFIX/Static/ftd3xx.lib
