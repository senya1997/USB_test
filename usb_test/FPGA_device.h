#ifndef FPGA_DEVICE_H
#define FPGA_DEVICE_H

#include <QFile>
#include <QTextStream>

#include <QThread>
#include <QObject>

#include "defines.h"

#include "USB_device.h"

class FPGA_device : public QObject
{
    Q_OBJECT

    public:
           FPGA_device();
           ~FPGA_device();

           bool Initialize(QString RbfFileName); // Начальная инициализация устройства

           bool StartTest(QString HexFileName);

           bool SetDepParameters();

    signals:
           void UpdLog(QString log_str);
           void UpdProgBar(int prog_value);

           bool GetDataWidth();

           void ShowMsg(QString title, QString msg);

    private slots:
           void UpdLogBridge(QString log_str);
           void UpdProgBarBridge(int prog_value);

           void ShowMsgBridge(QString title, QString msg);

    private:
           USB_device *usb_device = nullptr;

           //int prog_value;

           USHORT CurrFPGAVer;
};

#endif
