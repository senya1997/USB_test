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
    FT_DEVICE_LIST_INFO_NODE *ptrInfo;
    FT_60XCONFIGURATION oConfigurationData;

    int desc_size;

    bool isCorrected;

    emit UpdLog("USB_device::FT600_Connect: Start");

OPEN:
    prog_value = 0;
    emit UpdProgBar(0);

    ft600_drv->Cleanup();

// Проверка наличия нашего USB устройство,

    ft600_drv->GetDevicesInfoList(&ptrInfo);

    desc_size = 0;
    while(ptrInfo->Description[desc_size] != 0x00) ++desc_size;
    ++desc_size;

    char desc[desc_size];

    for(int i = 0; i < desc_size; ++i)
        desc[i] = ptrInfo->Description[i];

    if(!ft600_drv->Initialize(EOPEN_BY_DESC, (PVOID)desc, (PVOID)g_chSerialNumber))
    {
        emit UpdLog("USB_device::FT600_Connect: Connect error initialization");
        return false;
    }

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_CONNECT);

//Читаем конфигурацию
    isCorrected = false;

    if (!ft600_drv->ReadChipConfiguration(&oConfigurationData))
    {
        emit UpdLog("USB_device::FT600_Connect: Can not read FTDI configuration");
        return false;
    }

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_CONNECT);

//Проверяем конфигурацию
    if(QString::compare(desc, DEVICE_NAME120) == 0)
    {
        emit SetCycloneLEs(true);
        cyclone_LEs = true;
    }
    else if(QString::compare(desc, DEVICE_NAME080) == 0)
    {
        emit SetCycloneLEs(false);
        cyclone_LEs = false;
    }
    else
    {
        isCorrected = true;

        if(emit GetCycloneLEs())
        {
            cyclone_LEs = true;
            for(int i = 0; i < desc_size; ++i)
                desc[i] = DEVICE_NAME120[i];

            ft600_drv->SetStringDescriptors(oConfigurationData.StringDescriptors, sizeof(oConfigurationData.StringDescriptors),
                                            VENDOR_NAME, DEVICE_NAME120, g_chSerialNumber);
        }
        else
        {
            cyclone_LEs = false;
            for(int i = 0; i < desc_size; ++i)
                desc[i] = DEVICE_NAME080[i];

            ft600_drv->SetStringDescriptors(oConfigurationData.StringDescriptors, sizeof(oConfigurationData.StringDescriptors),
                                            VENDOR_NAME, DEVICE_NAME080, g_chSerialNumber);
        }
    }

    emit UpdFTDIDesc((QString)desc);

    if (oConfigurationData.OptionalFeatureSupport != (CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN | CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL))
    {//Session underrun
        oConfigurationData.OptionalFeatureSupport = CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN | CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL;
        isCorrected = true;
    }
    if (oConfigurationData.FIFOClock != CONFIGURATION_FIFO_CLK_66)
    {//FIFO clock
        oConfigurationData.FIFOClock = CONFIGURATION_FIFO_CLK_66;
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
        if (!ft600_drv->WriteChipConfiguration(&oConfigurationData))
        {
            emit UpdLog("USB_device::FT600_Connect: Failure configuration FTDI");
            return false;
        }

        QThread::msleep(4000); //1000 похоже не хватает
        //isCorrected = false; // ???

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

bool USB_device::FPGALoad(QString FileName)       //  Загрузка прошивки FPGA
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
    long* databuf_load = nullptr;

    databuf = new char[uChipZ64Size];
    Q_ASSERT(databuf);

    databuf_load = new long[uChipZ64Size * 16];
    Q_ASSERT(databuf_load);

    SrcFileChipZ64.read(databuf, uChipZ64Size);
    SrcFileChipZ64.close();

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_LOAD);








    quint32 DATA1_CLOCK0 = 0x40000000; //бит 30
    quint32 DATA1_CLOCK1 = 0xC0000000; //биты 30, 31
    quint32 DATA0_CLOCK0 = 0x00000000;
    quint32 DATA0_CLOCK1 = 0x80000000; //бит 31

//NConfig
    ft600_drv->WriteGPIO(1, 0);
        Sleep(2);
    ft600_drv->WriteGPIO(1, 1);

    quint32 k = 0;
        for(quint32 j = 0; j < uChipZ64Size; ++j)
        {
            for(quint32 i = 0; i < 8; ++i)
            {
                if((databuf[j] >> i) & 1)
                {
                    databuf_load[k++] = DATA1_CLOCK0;  //data = 1 clock = 0
                    databuf_load[k++] = DATA1_CLOCK1;  //data = 1 clock = 1
                }
                else
                {
                    databuf_load[k++] = DATA0_CLOCK0;  //data = 0 clock = 0
                    databuf_load[k++] = DATA0_CLOCK1;  //data = 0 clock = 1
                }
        }
    }








    ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)databuf_load, uChipZ64Size * 16 * 4, &ulActualBytesWritten, NULL);
    if (ftStatus != FT_OK)
    {
        emit UpdLog("USB_device::FPGALoad: Function WritePipe failed (check Power)");
        return false;
    }

    prog_value++;
    emit UpdProgBar(prog_value*100/PROG_LOAD);

    delete[] databuf;
    delete[] databuf_load;

    emit UpdLog("USB_device::FPGALoad: Successful load '.rbf' in FPGA");

    return true;
}

// Запись в устройство данных по адресу.
bool USB_device::UsbWrite(UCHAR* DataBuff, int BNum)
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
bool USB_device::UsbReadBuff(unsigned	Readchars,				//Сколько считать
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

    USB_busy = false;

    if (ftStatus == FT_OK)
    {
        emit UpdLog("USB_device::UsbReadBuff: Success read data");
        return true;
    }
    else
    {
        emit UpdLog("USB_device::UsbReadBuff: Failure read data");
        return false;
    }
}

bool USB_device::getUSB_busy() const
{
    return USB_busy;
}

bool USB_device::getCyclone_LEs() const
{
    return cyclone_LEs;
}
