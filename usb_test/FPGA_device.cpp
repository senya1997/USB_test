#include <QFile>
#include <QThread>
#include <QObject>

#include "FPGA_reg.h"

#define ZD4BU2018_CLOCK_FREQUENCY_MHZ       50										//  Частота тактового генератора в Мгц

using USHORT = unsigned short;
using UINT = unsigned int;

class FPGA_device
{
    public:
           FPGA_device()
           {
                usb_device = new USB_device();
           }

           ~FPGA_device()
           {
               if (usb_device)
               {
                   delete usb_device;
                   usb_device = nullptr;
               }
           }

           // Начальная инициализация устройства
           bool Initialize(QString RbfFileName)
           {
               if (!usb_device->FT600_Connect())
                   return false;

               if (!usb_device->FPGALoad(RbfFileName, &CurrFPGAVer))
                   return false;

               UINT Wr_Data;							//	Массив, загружаемый в контроллер

               Wr_Data = 0;
               if (!usb_device->UsbWrite(Adr_ADCch_PW21, &Wr_Data, 2, false))        //	0 channel AFE is PW2_1 Channel
               {
                   return false;
               }
               Wr_Data = 1;
               if (!usb_device->UsbWrite(Adr_ADCch_PW22, &Wr_Data, 2, false))        //	1 channel AFE is PW2_2 Channel
               {
                   return false;
               }

               // Устанавливаем начальные значения параметров
               m_PWPulseTime_us    = 2;

               return true;
           }

           // Загрузка значения длительности импульса (в микросекундах)
           bool SetPWPulseTime_us (USHORT PWPulseTime_us)
           {
               UINT Wr_Data;							//	Массив, загружаемый в контроллер

               while (usb_device->USB_busy);
               Wr_Data = PWPulseTime_us*2;
               if (!usb_device->UsbWrite(Adr_PTrN, &Wr_Data, 2, false))          //	Stop
               {
                   return false;
               }
               m_PWPulseTime_us = PWPulseTime_us;
               // Пересчитываем и записываем все зависимые параметры
               SetDepParameters();
               return true;
           }

           // Запуск работы
           bool Start()
           {
               UINT Wr_Data;							//	Массив, загружаемый в контроллер
               Wr_Data = 0;
               if (!usb_device->UsbWrite(Adr_DBufReset, &Wr_Data, 2, false))			//	Reset FIFO
               {
                   return false;
               }

               Wr_Data = 1;
               if (!usb_device->UsbWrite(Adr_Start_CA, &Wr_Data, 2, false))          //	Start
               {
                   return false;
               }
               return true;
           }

           // Остановка работы
           bool Stop()
           {
               UINT Wr_Data;							//	Массив, загружаемый в контроллер

               while (usb_device->USB_busy);
               Wr_Data = 0;
               if (!usb_device->UsbWrite(Adr_Start_CA, &Wr_Data, 2, false))          //	Stop
               {
                   return false;
               }
               return true;
           }

    private:
           USB_device *usb_device = nullptr;

           USHORT CurrFPGAVer;
           USHORT m_PWPulseTime_us;                            //  Длительность импульса передатчика в микросекундах

           bool SetDepParameters()                            // Загрузка зависимых параметров
           {
               UINT Wr_Data;							//	Массив, загружаемый в контроллер

           // Время между окончанием импульса и началом приёма
               uint16_t PWMinDepth_us = (uint16_t)(123456);
               while (usb_device->USB_busy);

               Wr_Data = (PWMinDepth_us+m_PWPulseTime_us) * ZD4BU2018_CLOCK_FREQUENCY_MHZ;
               if (!usb_device->UsbWrite(Adr_DataStartD, &Wr_Data, 2, false))
               {
                   return false;
               }

               return true;
           }
};
