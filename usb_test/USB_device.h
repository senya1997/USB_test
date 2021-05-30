#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include <QFile>
#include <QThread>

#include "defines.h"
#include "ft600_drvinterface.h"

class USB_device : public QObject
{
    Q_OBJECT

    public:
        USB_device();
        ~USB_device();

        bool FT600_Connect(void);
        bool FT600_DisConnect(void);

        inline bool isInitialized() const
            { return ft600_drv && ft600_drv->isInitialized(); }

        int SetFPGAPower(eFPGAPOWER ePwr); //  Управление питанием FPGA.
        bool FPGALoad(QString FileName); //  Загрузка прошивки FPGA

        bool UsbWrite(UCHAR* DataBuff, int BNum);
        bool UsbReadBuff(unsigned Readchars, ULONG* p_uLengthTransfered, UCHAR* ReadBuff); // Чтение данных из устройства

        bool getUSB_busy() const;

        bool getCyclone_LEs() const;

signals:
        void UpdLog(QString log_str);
        void UpdProgBar(int prog_value);

        void UpdFTDIDesc(QString desc);

        bool GetCycloneLEs();
        void SetCycloneLEs(bool cyclone_LEs);

        void ShowMsg(QString title, QString msg);

    private:
        FT600_DrvInterface *ft600_drv = nullptr;

        bool USB_busy = false;
        bool cyclone_LEs = false;

        int prog_value = 0;
};

#endif
