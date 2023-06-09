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

        eConnInfo FT600_Connect(void);
        bool FT600_Disconnect(void);
        bool FT600_Reconfig(bool force_reconf, bool ui_cyclone_LEs); // this method call 'FT600_Connect'

        eProgInfo FPGAProg(QString FileName, bool ui_data_width); //  Загрузка прошивки FPGA

        bool UsbWriteBuff(UCHAR* DataBuff, int BNum);
        bool UsbReadBuff(unsigned Readchars, ULONG* p_uLengthTransfered, UCHAR* ReadBuff); // Чтение данных из устройства

        inline bool isInitialized() const
            { return ft600_drv && ft600_drv->isInitialized(); }

    // setter/getter:
        bool getUSB_busy() const;
        bool getCyclone_LEs() const;

signals:
        void UpdLog(QString log_str);
        void UpdProgBar(int prog_value);

        void UpdFTDIDesc(QString desc);

        void SetCycloneLEs(bool cyclone_LEs);

    private:
        FT600_DrvInterface *ft600_drv = nullptr;

        bool USB_busy = false;
        bool cyclone_LEs = false; // true - 120k, false - 80k

        int prog_value = 0;

        int SetFPGAPower(eFPGAPOWER ePwr); //  Управление питанием FPGA
};

#endif
