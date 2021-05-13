#include "USB_device.h"

USB_device::USB_device()
{
    ft600_drv = new FT600_DrvInterface();
}

USB_device::~USB_device()
{
    if (ft600_drv)
    {
        delete ft600_drv;
        ft600_drv = nullptr;
    }
}

bool USB_device::FT600_Connect(void) //Соединение с FT600, инициализация FT600
{
OPEN:
    prog_value = 0;
    emit UpdProgBar(0);

    ft600_drv->Cleanup();

    emit UpdLog("USB_device::FT600_Connect: Start");

// Проверка наличия нашего USB устройство,
    bool bResult = ft600_drv->Initialize(EOPEN_BY_DESC, (PVOID)ZDOPP_DEVICE_NAME, (PVOID)g_chSerialNumber);

    if (!bResult)
    {
        emit UpdLog("USB_device::FT600_Connect: Connect error initialization");
        return false;
    }

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_CONNECT);

//Проверяем и при необходимости корректируем конфигурацию FTDI
    FT_60XCONFIGURATION oConfigurationData;
    bool isCorrected = false;

//Читаем конфигурацию
    if (!ft600_drv->ReadChipConfiguration(&oConfigurationData))
    {
        emit UpdLog("USB_device::FT600_Connect: Can not read FTDI configuration");
        return false;
    }

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_CONNECT);

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
        {
            emit UpdLog("USB_device::FT600_Connect: Failure configuration FTDI");
            return false;
        }

        QThread::msleep(4000);  //1000 похоже не хватает
        isCorrected = false; // ???

        goto OPEN; // reconfig
    }

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_CONNECT);

    SetFPGAPower(Pwr_ON);

    emit UpdLog("USB_device::FT600_Connect: Successful, FPGA power is ON");

    USB_busy = false;

    return true;
}

bool USB_device::FT600_DisConnect(void)                           //Отсоединение от FT600
{
    if (ft600_drv)
    {
        SetFPGAPower(Pwr_OFF);
        delete ft600_drv;
        ft600_drv = nullptr;

        emit UpdLog("USB_device::FT600_DisConnect: Success disconnect FTDI");
        return true;
    }
    else
    {
        emit UpdLog("USB_device::FT600_DisConnect: Failure to disconnect FTDI");
        return false;
    }
}

int USB_device::SetFPGAPower(eFPGAPOWER ePwr)                   //  Управление питанием FPGA.
{
    if (ePwr == Pwr_ON) ft600_drv->WriteGPIO(2, 3);// вкл питание
    else ft600_drv->WriteGPIO(2, 0);// выкл питание

    Sleep(100);
    return 0;
}

bool USB_device::FPGALoad(QString FileName, USHORT *FPGAVer)       //  Загрузка прошивки FPGA
{
    prog_value = 0;
    emit UpdProgBar(0);

    FT_STATUS ftStatus;
    ULONG ulActualBytesWritten = 0;
    ULONG uChipZ64Size = 0;

    emit UpdLog("USB_device::FPGALoad: Start");

    QFile SrcFileChipZ64(FileName);

    if (!SrcFileChipZ64.open(QIODevice::ReadOnly))
    {
        emit ShowMsg("Wrong Path", "File " + FileName + " not opened");
        emit UpdLog("USB_device::FPGALoad: '.rbf' file not openned");
        return false;
    }

    prog_value++;
    emit UpdProgBar((int)((float)prog_value*100/PROG_LOAD));

    uChipZ64Size = (ULONG)SrcFileChipZ64.size(); //GetLength();

    char * databuf = nullptr;
    short* databuf_load = nullptr;

    databuf = new char[uChipZ64Size];
    Q_ASSERT(databuf);

    databuf_load = new short[uChipZ64Size * 16];
    Q_ASSERT(databuf_load);

    SrcFileChipZ64.read(databuf, uChipZ64Size);
    SrcFileChipZ64.close();

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_LOAD);

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
        }
    }

    ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)databuf_load, uChipZ64Size * 32, &ulActualBytesWritten, NULL);
    if (ftStatus != FT_OK)
    {
        emit UpdLog("USB_device::FPGALoad: Function WritePipe failed (check Power)");
        return false;
    }

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_LOAD);

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

    emit UpdLog("USB_device::FPGALoad: Successful load '.rbf' in FPGA");

    return true;
}

// Запись в устройство данных по адресу.
bool USB_device::UsbWrite(UINT* DataBuff, int BNum)
{
    FT_STATUS ftStatus = FT_OK;             //	Статус операции записи

    ULONG ulActualBytesWritten = 0;			//	Колличество записанных байтов

    USB_busy = true;

    emit UpdLog("USB_device::UsbWrite: Start send data array to FPGA");

    ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)DataBuff, BNum, &ulActualBytesWritten, NULL);	//	Загрузка массива данных в ПЛИС

    USB_busy = false;

    if (ftStatus == FT_OK)
    {
        emit UpdLog("USB_device::UsbWrite: Success write data");
        return true;
    }
    else
    {
        emit UpdLog("USB_device::UsbWrite: Failure write data");
        return false;
    }
}

// old version with ADDR and read request:
// Если WrType = true, то в устройство записывается запрос чтения по Addr
/*
bool USB_device::UsbWrite(BYTE addr, UINT* DataBuff, int BNum, bool WrType)
{
    FT_STATUS ftStatus = FT_OK;             //	Статус операции записи
    FT_STATUS ftStatus1 = FT_OK;			//	Статус операции записи

    ULONG ulActualBytesWritten = 0;			//	Колличество записанных байтов

    USB_busy = true;

    if (WrType == false)
    {
        emit UpdLog("USB_device::UsbWrite: Start send data array to FPGA");

        ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)(&addr), 1, &ulActualBytesWritten, NULL);	//	Загрузка запроса записи и адреса регистра в ПЛИС
        ftStatus1 = ft600_drv->WritePipe(0x02, (PUCHAR)DataBuff, BNum, &ulActualBytesWritten, NULL);	//	Загрузка массива данных в ПЛИС
    }
    else
    {
        emit UpdLog("USB_device::UsbWrite: Start send read request to FPGA");

        BYTE Request = addr | 0x80;
        ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)(&Request), 1, &ulActualBytesWritten, NULL);			//	Загрузка запроса чтения и адреса регистра в ПЛИС
    }

    USB_busy = false;

    if (ftStatus == FT_OK && ftStatus1 == FT_OK)
    {
        emit UpdLog("USB_device::UsbWrite: Success write data");
        return true;
    }
    else
    {
        emit UpdLog("USB_device::UsbWrite: Failure write data");
        return false;
    }
}
*/

//Чтение порции данных по USB
void USB_device::UsbReadBuff(unsigned	Readchars,				//Сколько считать
                             ULONG*		p_uLengthTransfered,	//Сколько считать получилось
                             UCHAR*     ReadBuff)               //Куда считывать
{
    FT_STATUS ftStatus = FT_OK;						//	Статус операции записи

    //	ULONG RdCnt = 0;							//	Колличество байтов считанных за одну операцию чтения

    //	If lpOverlapped is NULL, FT_ReadPipe operates synchronously, that is, it returns only when
    //	the transfer has been completed.

    Q_ASSERT(ft600_drv);
    Q_ASSERT(ReadBuff);

    USB_busy = true;

    ftStatus = ft600_drv->ReadPipe(0x82, ReadBuff, Readchars, p_uLengthTransfered, NULL);

    if (ftStatus == FT_OK)
    {
        emit UpdLog("USB_device::UsbReadBuff: Success read data");
    }
    else
    {
        emit UpdLog("USB_device::UsbReadBuff: Failure read data");
    }

    USB_busy = false;
}

bool USB_device::getUSB_busy() const
{
    return USB_busy;
}

/*
void USB_device::setUSB_busy(bool value)
{
    USB_busy = value;
}
*/
