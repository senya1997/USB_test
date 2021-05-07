#include <QFile>
#include <QThread>
#include "header/zd4bu_2018_usbprot.h"
#include "header/zd4bu_2018_fpgareg.h"

zd4bu_2018_usbprot::zd4bu_2018_usbprot()
{
    m_pDriver = new FT600_DrvInterface();
        Q_ASSERT(m_pDriver);
}

void zd4bu_2018_usbprot::SetLogger(logger* log)
{
    m_DbgLog = log;
    IsLoggerInitted = true;
}

bool zd4bu_2018_usbprot::FT600_Connect(void)
{
    OPEN:
    Q_ASSERT(m_pDriver);
    m_pDriver->Cleanup();
    // Проверка наличия нашего USB устройство,
    bool bResult = m_pDriver->Initialize(EOPEN_BY_DESC, (PVOID)ZDOPP_DEVICE_NAME, (PVOID)g_chSerialNumber);
    if (!bResult)
    {
        if (IsLoggerInitted)
        {
            QString dbgStr = "ZD4BU_2018::FT600_Connect Connect Error initialization" + m_pDriver->StatusStr;
            m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
        }
        return false; //Отваливаем
    }
    //Проверяем и при необходимости корректируем конфигурацию FTDI
    FT_60XCONFIGURATION oConfigurationData;
    bool isCorrected = false;
    //Читаем конфигурацию
    if (!m_pDriver->ReadChipConfiguration(&oConfigurationData))
    {
        //не получилось
        if (IsLoggerInitted)
        {
            QString dbgStr = "ZD4BU_2018::FT600_Connect Can not read FTDI configuration!";
            m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
        }
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
        m_pDriver->SetStringDescriptors(oConfigurationData.StringDescriptors, sizeof(oConfigurationData.StringDescriptors),
            VENDOR_NAME,
            ZDOPP_DEVICE_NAME,
            g_chSerialNumber //"v1"
        );

        if (!m_pDriver->WriteChipConfiguration(&oConfigurationData))
        {   //прошивка не задалась
            if (IsLoggerInitted)
            {
                QString dbgStr = "ZD4BU_2018::FT600_Connect Failure configuration FTDI Chip!!";
                m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
            }
            return false;
        }
        QThread::msleep(4000);  //1000 похоже не хватает
        isCorrected = false;
        goto OPEN;
    }

    SetFPGAPower(Pwr_ON);
    if (IsLoggerInitted)
    {
        QString dbgStr = "ZD4BU_2018::FT600_Connect Siccesfully...";
        m_DbgLog->MsgOutput(QtMsgType::QtInfoMsg,dbgStr);
    }
    USB_busy = false;
    return true;
}

void zd4bu_2018_usbprot::FT600_DisConnect(void)
{
    if (m_pDriver)
    {
        SetFPGAPower(Pwr_OFF);
        delete m_pDriver;
        m_pDriver = nullptr;
        if (IsLoggerInitted)
        {
            QString dbgStr = "ZD4BU_2018::FT600_DisConnect Succesfully ...";
            m_DbgLog->MsgOutput(QtMsgType::QtInfoMsg,dbgStr);
        }
    }
    else
    {
        if (IsLoggerInitted)
        {
            QString dbgStr = "ZD4BU_2018::FT600_DisConnect Device not Connected !!!";
            m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
        }
    }
}

//Управление питанием. Включение (1) Выключение (0)
///////////////////////////////////////////////////////////////////////////////////
int zd4bu_2018_usbprot::SetFPGAPower(eFPGAPOWER ePwr)
{
    if (ePwr == Pwr_ON)
        m_pDriver->WriteGPIO(2, 3);// вкл питание
    else
        m_pDriver->WriteGPIO(2, 0);// выкл питание

    Sleep(100);
    return 0;
}

//Загрузка альтеры из файла
////////////////////////////////////////////////////////////////////////////////////
bool zd4bu_2018_usbprot::FPGALoad(QString FileName,USHORT *FPGAVer)
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
    m_pDriver->WriteGPIO(1, 0);
    Sleep(2);
    m_pDriver->WriteGPIO(1, 1);

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

    ftStatus = m_pDriver->WritePipe(0x02, (PUCHAR)databuf_load, uChipZ64Size * 32, &ulActualBytesWritten, NULL);
    if (ftStatus != FT_OK)
    {
        if (IsLoggerInitted)
        {
            QString dbgStr = "ZD4BU_2018::FPGALoad Function WritePipe failed (Check Power) !!!";
            m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
        }
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

bool zd4bu_2018_usbprot::UsbWrite(BYTE addr, UINT* DataBuff, int BNum, bool WrType)
{
    FT_STATUS ftStatus = FT_OK;             //	Статус операции записи
    FT_STATUS ftStatus1 = FT_OK;			//	Статус операции записи
    ULONG ulActualBytesWritten = 0;			//	Колличество записанных байтов
    USB_busy = true;
    if (WrType == false)
    {
        ftStatus = m_pDriver->WritePipe(0x02, (PUCHAR)(&addr), 1, &ulActualBytesWritten, NULL);	//	Загрузка запроса записи и адреса регистра в ПЛИС
        ftStatus1 = m_pDriver->WritePipe(0x02, (PUCHAR)DataBuff, BNum, &ulActualBytesWritten, NULL);	//	Загрузка массива данных в ПЛИС
    }
    else
    {
        BYTE Request = addr | 0x80;
        ftStatus = m_pDriver->WritePipe(0x02, (PUCHAR)(&Request), 1, &ulActualBytesWritten, NULL);			//	Загрузка запроса чтения и адреса регистра в ПЛИС
    }
    USB_busy = false;
    if (ftStatus == FT_OK && ftStatus1 == FT_OK)
    {
        if (IsLoggerInitted)
        {
            //QString dbgStr = "ZD4BU_2018::UsbWrite Addr: " + QString::number(addr) + " Data: " + QString::number(DataBuff[0]) + " succesfully ...";
            //m_DbgLog->MsgOutput(QtMsgType::QtInfoMsg,dbgStr);
        }
        return true;
    }
    else
    {
        if (IsLoggerInitted)
        {
            //QString dbgStr = "ZD4BU_2018::UsbWrite Addr: " + QString::number(addr) + " Data: " + QString::number(DataBuff[0]) + " error !!!";
            //m_DbgLog->MsgOutput(QtMsgType::QtFatalMsg,dbgStr);
        }
        return false;
    }
}

//Чтение порции данных по USB
////////////////////////////////////////////////////////////////////////////////////////////
void zd4bu_2018_usbprot::UsbReadBuff(unsigned	Readchars,				//Сколько считать
                             ULONG*		p_uLengthTransfered,	//Сколько считать получилось
                             UCHAR*     ReadBuff)               //Куда считывать
{
    FT_STATUS ftStatus = FT_OK;						//	Статус операции записи

    //	ULONG RdCnt = 0;							//	Колличество байтов считанных за одну операцию чтения

    //	If lpOverlapped is NULL, FT_ReadPipe operates synchronously, that is, it returns only when
    //	the transfer has been completed.

    Q_ASSERT(m_pDriver);
    Q_ASSERT(ReadBuff);

    USB_busy = true;
    ftStatus = m_pDriver->ReadPipe(0x82, ReadBuff, Readchars, p_uLengthTransfered, NULL);
    USB_busy = false;
    //	Q_ASSERT(ftStatus == FT_OK);   //Не надо проверять
}
