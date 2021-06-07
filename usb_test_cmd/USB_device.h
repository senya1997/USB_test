#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include <fstream>
#include <chrono>
#include <thread>

#include "defines.h"
#include "ft600_drvinterface.h"

class USB_device
{
    public:
        USB_device();
        ~USB_device();

        eConnInfo FT600_Connect(void);
        bool FT600_Disconnect(void);
        bool FT600_Reconfig(bool force_reconf, bool ui_cyclone_LEs); // this method call 'FT600_Connect'

        eProgInfo FPGAProg(string FileName, bool ui_data_width); //  Загрузка прошивки FPGA

        bool UsbWriteBuff(UCHAR* DataBuff, int BNum);
        bool UsbReadBuff(unsigned Readchars, ULONG* p_uLengthTransfered, UCHAR* ReadBuff); // Чтение данных из устройства

        inline bool isInitialized() const
            { return ft600_drv && ft600_drv->isInitialized(); }

    // setter/getter:
        bool getCyclone_LEs() const;

    private:
        FT600_DrvInterface *ft600_drv = nullptr;

        bool cyclone_LEs = false; // true - 120k, false - 80k

        int prog_value = 0;

        int SetFPGAPower(eFPGAPOWER ePwr); //  Управление питанием FPGA
};

#endif
