#include <QFile>
#include <QThread>

#include "ft600_drvinterface.h"

using BYTE = char;

using UCHAR = unsigned char;
using USHORT = unsigned short;
using UINT = unsigned int;
using ULONG = unsigned long;

//Устройство, которое мы ищем (должно быть зашито в FTDI 600)
static const char ZDOPP_DEVICE_NAME[]			= "ZDopp USB3.0 v1";
static const char* g_chSerialNumber             = "000000000001";
static const char VENDOR_NAME[]					= "JS Zaslon";

enum eFPGAPOWER {
    Pwr_OFF,
    Pwr_ON
};

class USB_device
{
    public:
        USB_device()
        {
            ft600_drv = new FT600_DrvInterface();
        }

        bool FT600_Connect(void)                               //Соединение с FT600, инициализация FT600
        {
            OPEN:

            ft600_drv->Cleanup();
            // Проверка наличия нашего USB устройство,
            bool bResult = ft600_drv->Initialize(EOPEN_BY_DESC, (PVOID)ZDOPP_DEVICE_NAME, (PVOID)g_chSerialNumber);
            if (!bResult)
            {
                return false; //Отваливаем
            }

            //Проверяем и при необходимости корректируем конфигурацию FTDI
            FT_60XCONFIGURATION oConfigurationData;
            bool isCorrected = false;
            //Читаем конфигурацию
            if (!ft600_drv->ReadChipConfiguration(&oConfigurationData))
            {
                return false; //Отваливаем
            }

            //Проверяем конфигурацию
            if (oConfigurationData.OptionalFeatureSupport != (CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN | CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL))
            {//Session underrun
                oConfigurationData.OptionalFeatureSupport = CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN | CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL;
                isCorrected = true;
            }
            if (oConfigurationData.FIFOClock != CONFIGURATION_FIFO_CLK_100) //CONFIGURATION_FIFO_CLK_66)
            {//FIFO clock
                oConfigurationData.FIFOClock = CONFIGURATION_FIFO_CLK_100;  //CONFIGURATION_FIFO_CLK_66;
                isCorrected = true;
            }
            if (oConfigurationData.FIFOMode != CONFIGURATION_FIFO_MODE_245)
            {//FIFO mode
                oConfigurationData.FIFOMode = CONFIGURATION_FIFO_MODE_245;
                isCorrected = true;
            }
            if (oConfigurationData.ChannelConfig != CONFIGURATION_CHANNEL_CONFIG_1)
            {//Num of pipes
                oConfigurationData.ChannelConfig = CONFIGURATION_CHANNEL_CONFIG_1;
                isCorrected = true;
            }

            //Пишем, если корректировалась
            if (isCorrected)
            {
                //vendor/device name
                ft600_drv->SetStringDescriptors(oConfigurationData.StringDescriptors, sizeof(oConfigurationData.StringDescriptors),
                    VENDOR_NAME,
                    ZDOPP_DEVICE_NAME,
                    g_chSerialNumber //"v1"
                );

                if (!ft600_drv->WriteChipConfiguration(&oConfigurationData))
                {   //прошивка не задалась
                    return false;
                }

                QThread::msleep(4000);  //1000 похоже не хватает
                isCorrected = false;
                goto OPEN;
            }

            SetFPGAPower(Pwr_ON);

            USB_busy = false;
            return true;
        }

        void FT600_DisConnect(void)                           //Отсоединение от FT600
        {
            if (ft600_drv)
            {
                SetFPGAPower(Pwr_OFF);
                delete ft600_drv;
                ft600_drv = nullptr;
            }
            else
            {

            }
        }

        inline bool isInitialized() const { return ft600_drv && ft600_drv->isInitialized(); }

        bool FPGALoad(QString FileName, USHORT *FPGAVer)       //  Загрузка прошивки FPGA
        {
            FT_STATUS ftStatus;
            ULONG ulActualBytesWritten = 0;
            ULONG uChipZ64Size = 0;

            QFile SrcFileChipZ64(FileName);
            if (!SrcFileChipZ64.open(QIODevice::ReadOnly))
            {
                if (IsLoggerInitted)
                {
                    QString dbgStr = "ZD4BU_2018::FPGALoad File: " + FileName + " not openned !!!";
                    m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
                }
                return false;
            }

            uChipZ64Size = (ULONG)SrcFileChipZ64.size(); //GetLength();

            char * databuf = nullptr;
            short* databuf_load = nullptr;

            databuf = new char[uChipZ64Size];
            Q_ASSERT(databuf);

            databuf_load = new short[uChipZ64Size * 16];
            Q_ASSERT(databuf_load);

            SrcFileChipZ64.read(databuf, uChipZ64Size);
            SrcFileChipZ64.close();

            //NConfig
            ft600_drv->WriteGPIO(1, 0);
            Sleep(2);
            ft600_drv->WriteGPIO(1, 1);

            unsigned k = 0;

            for (unsigned j = 0; j < uChipZ64Size; j++)
            {
                for (unsigned i = 0; i < 8; i++)
                {
                    if ((databuf[j] >> i) & 1)
                    {
                        databuf_load[k++] = 0x400;//data = 1 clock = 0
                        databuf_load[k++] = 0xC00;//data = 1 clock = 1
                    }
                    else
                    {
                        databuf_load[k++] = 0x0;//data = 0 clock = 0
                        databuf_load[k++] = 0x800;//data = 0 clock = 1
                    }
                }//i
            }//j

            ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)databuf_load, uChipZ64Size * 32, &ulActualBytesWritten, NULL);
            if (ftStatus != FT_OK)
            {
                return false;
            }
            delete[] databuf;
            delete[] databuf_load;

            //Делаем запись/чтение в регистр Adr_Test для первичной проверки работоспосбная ли прошивка загрузилась

            /*UINT Test_Data = 0xC006;                        //	Значение тестового регистра
            UINT Wr_Data;                                   //	Массив, загружаемый в тестовый регистр

            USHORT Data_pnt[4];
            UINT RdCnt;                                     //	Текущее колличество слов, считанное из ПЛИС

            Wr_Data = 0;
            UsbWrite(Adr_AbortPipe, &Wr_Data, 2, false);    //	AbortPipe
            Wr_Data = Test_Data;
            UsbWrite(Adr_Test, &Wr_Data, 2, false);			//	Запись значения в тестовый регистр
            Wr_Data = 1;
            UsbWrite(Adr_GetInfo, &Wr_Data, 2, false);		//	Запись GetInfo

            UsbReadBuff(8, (ULONG*)&RdCnt,(UCHAR*)(Data_pnt));  //  Читаем данные из FPGA

            if (Data_pnt[1] != Test_Data)
            {
                if (IsLoggerInitted)
                {
                    QString dbgStr = "ZD4BU_2018::FPGALoad Error *.rbf file! Write to Adr_Test: " + QString::number(Test_Data) + "Read from Adr_Test: " + QString::number(Data_pnt[2]) + " !!!";
                    m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
                }
                return false;
            }
            if (IsLoggerInitted)
            {
                QString dbgStr = "ZD4BU_2018::FPGALoad FPGALoad Succesfully... FPGAVer = " + QString::number(Data_pnt[0]);
                m_DbgLog->MsgOutput(QtMsgType::QtInfoMsg,dbgStr);
            }
            *FPGAVer = Data_pnt[0];*/
            return true;
        }

        // Запись в устройство данных по адресу.
        // Если WrType = true, то в устройство записывается запрос чтения по Addr
        bool UsbWrite(BYTE addr, UINT* DataBuff, int BNum, bool WrType)
        {
            FT_STATUS ftStatus = FT_OK;             //	Статус операции записи
            FT_STATUS ftStatus1 = FT_OK;			//	Статус операции записи
            ULONG ulActualBytesWritten = 0;			//	Колличество записанных байтов
            USB_busy = true;
            if (WrType == false)
            {
                ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)(&addr), 1, &ulActualBytesWritten, NULL);	//	Загрузка запроса записи и адреса регистра в ПЛИС
                ftStatus1 = ft600_drv->WritePipe(0x02, (PUCHAR)DataBuff, BNum, &ulActualBytesWritten, NULL);	//	Загрузка массива данных в ПЛИС
            }
            else
            {
                BYTE Request = addr | 0x80;
                ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)(&Request), 1, &ulActualBytesWritten, NULL);			//	Загрузка запроса чтения и адреса регистра в ПЛИС
            }
            USB_busy = false;
            if (ftStatus == FT_OK && ftStatus1 == FT_OK)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        // Чтение данных из устройства
        void UsbReadBuff(unsigned Readchars, ULONG* p_uLengthTransfered, UCHAR* ReadBuff)
        {
            FT_STATUS ftStatus = FT_OK;						//	Статус операции записи

            //	ULONG RdCnt = 0;							//	Колличество байтов считанных за одну операцию чтения

            //	If lpOverlapped is NULL, FT_ReadPipe operates synchronously, that is, it returns only when
            //	the transfer has been completed.

            Q_ASSERT(ft600_drv);
            Q_ASSERT(ReadBuff);

            USB_busy = true;
            ftStatus = ft600_drv->ReadPipe(0x82, ReadBuff, Readchars, p_uLengthTransfered, NULL);
            USB_busy = false;
            //	Q_ASSERT(ftStatus == FT_OK);   //Не надо проверять
        }

        bool USB_busy;
    private:
        FT600_DrvInterface  *ft600_drv = nullptr;

        int SetFPGAPower(eFPGAPOWER ePwr)                   //  Управление питанием FPGA.
        {
            if (ePwr == Pwr_ON)
                ft600_drv->WriteGPIO(2, 3);// вкл питание
            else
                ft600_drv->WriteGPIO(2, 0);// выкл питание

            Sleep(100);
            return 0;
        }
};
