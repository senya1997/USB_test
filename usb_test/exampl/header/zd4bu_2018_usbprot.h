#ifndef ZD4BU_2018_USBPROT_H
#define ZD4BU_2018_USBPROT_H

#include "ft600_drvinterface.h"
#include "../src/logger.h"

//Устройство, которое мы ищем (должно быть зашито в FTDI 600)
static const char ZDOPP_DEVICE_NAME[]			= "ZDopp USB3.0 v1";
static const char* g_chSerialNumber             = "000000000001";
static const char VENDOR_NAME[]					= "JS Zaslon";

enum eFPGAPOWER {
    Pwr_OFF		= 0,
    Pwr_ON		= 1
};

class zd4bu_2018_usbprot
{
    public:
        zd4bu_2018_usbprot();
        bool FT600_Connect(void);                               //Соединение с FT600, инициализация FT600
        void FT600_DisConnect(void);                            //Отсоединение от FT600
        inline bool isInitialized() const { return m_pDriver && m_pDriver->isInitialized(); }
        bool FPGALoad(QString FileName, USHORT *FPGAVer);       //  Загрузка прошивки FPGA
        // Запись в устройство данных по адресу.
        // Если WrType = true, то в устройство записывается запрос чтения по Addr
        bool UsbWrite(BYTE addr, UINT* DataBuff, int BNum, bool WrType);
        // Чтение данных из устройства
        void UsbReadBuff(unsigned Readchars, ULONG* p_uLengthTransfered, UCHAR* ReadBuff);
        void SetLogger(logger*);                            //  Устанавливает класс для записи отладочных сообщений
        bool USB_busy;
    private:
        FT600_DrvInterface  *m_pDriver = nullptr;
        int SetFPGAPower(eFPGAPOWER ePwr);                   //  Управление питанием FPGA.
        logger* m_DbgLog;
        bool IsLoggerInitted;

};

#endif // ZD4BU_2018_USBPROT_H
