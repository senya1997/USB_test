#ifndef FPGA_DEVICE_H
#define FPGA_DEVICE_H

#include <QFile>
#include <QTextStream>

#include <QThread>
//#include <QObject>

//#include <QTimer>

#include "defines.h"
#include "USB_device.h"

class FPGA_device : public QObject
{
    Q_OBJECT

    public:
           FPGA_device();
           ~FPGA_device();

           eInitInfo Initialize(bool force_reconf, bool ui_cyclone_LEs); // Начальная инициализация устройства

           eProgInfo Program(QString RbfFileName, bool def_path, bool ui_data_width, bool ui_cyclone_LEs);

           eTestInfo StartTest(QString HexFileName, bool ui_data_width);

    signals:
           void UpdLog(QString log_str);
           void UpdProgBar(int prog_value);

           void SetDataWidth(bool data_width);
           void SetCycloneLEs(bool cyclone_LEs);

           void UpdRbfPath(QString rbf_path);
           void UpdFTDIDesc(QString desc);

           void UpdVerFPGA(QString ver);
           void UpdDateFPGA(QString date);

    private slots:
           void UpdLogBridge(QString log_str);
           void UpdProgBarBridge(int prog_value);

           void UpdFTDIDescBridge(QString desc);

           void SetCycloneLEsBridge(bool cyclone_LEs);

    private:
           USB_device *usb_device = nullptr;

           //int prog_value;

           USHORT CurrFPGAVer;

           //void ShowBuff();
};

#endif
