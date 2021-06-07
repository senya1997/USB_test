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

eConnInfo USB_device::FT600_Connect(void)
{
    int desc_size = 0;
    FT_DEVICE_LIST_INFO_NODE *ptrInfo;

    emit UpdLog("USB_device::FT600_Connect: Start");

    USB_busy = true;

// check FTDI connect:
    ft600_drv->Cleanup();
    ft600_drv->GetDevicesInfoList(&ptrInfo);

    while(ptrInfo->Description[desc_size] != 0x00) ++desc_size; // calc descriptor string size in common info about connected FTDI
    ++desc_size;

    char desc[desc_size];

    for(int i = 0; i < desc_size; ++i)
        desc[i] = ptrInfo->Description[i];

    if(!ft600_drv->Initialize(EOPEN_BY_DESC, (PVOID)desc, (PVOID)g_chSerialNumber))
    {
        emit UpdFTDIDesc("Init error");
        emit UpdLog("USB_device::FT600_Connect: Error initialization");
        return conn_err_init;
    }

// check descriptor:
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
        emit UpdFTDIDesc("Wrong descriptor");
        emit UpdLog("USB_device::FT600_Connect: Init succsess but wrong descriptor");

        USB_busy = false;

        return conn_wrong_desc;
    }

    emit UpdFTDIDesc((QString)desc);
    emit UpdLog("USB_device::FT600_Connect: Successful");

    USB_busy = false;

    return conn_ok;
}
bool USB_device::FT600_Disconnect(void)
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
bool USB_device::FT600_Reconfig(bool force_reconf, bool ui_cyclone_LEs)
{
    FT_60XCONFIGURATION oConfigurationData;
    eConnInfo ftdi_connect;

    bool isCorrected = false;
    bool in_force_reconf;

    emit UpdLog("USB_device::FT600_Reconfig: Start");

    prog_value = 0;
    emit UpdProgBar(0);

    ftdi_connect = FT600_Connect();

    if(ftdi_connect == conn_err_init)
        return false;
    else if((ftdi_connect == conn_wrong_desc) | force_reconf)
        in_force_reconf = true;
    else
        in_force_reconf = false;

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_RECONF));

// read configuration:
    if (!ft600_drv->ReadChipConfiguration(&oConfigurationData))
    {
        emit UpdLog("USB_device::FT600_Reconfig: Can not read FTDI configuration");
        return false;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_RECONF));

// change descriptor if needed:
    if(in_force_reconf)
    {
        isCorrected = true;

        if(ui_cyclone_LEs)
        {
            ft600_drv->SetStringDescriptors(oConfigurationData.StringDescriptors, sizeof(oConfigurationData.StringDescriptors), VENDOR_NAME, DEVICE_NAME120, g_chSerialNumber);
            cyclone_LEs = true;
        }
        else
        {
            ft600_drv->SetStringDescriptors(oConfigurationData.StringDescriptors, sizeof(oConfigurationData.StringDescriptors), VENDOR_NAME, DEVICE_NAME080, g_chSerialNumber);
            cyclone_LEs = false;
        }
    }

    if (oConfigurationData.OptionalFeatureSupport != FTDI_SESSION_UNDERRUN)
    {
        oConfigurationData.OptionalFeatureSupport = FTDI_SESSION_UNDERRUN;
        isCorrected = true;
    }
    if (oConfigurationData.FIFOClock != FTDI_FIFO_CLK)
    {
        oConfigurationData.FIFOClock = FTDI_FIFO_CLK;
        isCorrected = true;
    }
    if (oConfigurationData.FIFOMode != FTDI_FIFO_MODE)
    {
        oConfigurationData.FIFOMode = FTDI_FIFO_MODE;
        isCorrected = true;
    }
    if (oConfigurationData.ChannelConfig != FTDI_CHANNEL_CONFIG)
    {
        oConfigurationData.ChannelConfig = FTDI_CHANNEL_CONFIG;
        isCorrected = true;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_RECONF));

//Пишем, если корректировалась
    if (isCorrected)
    {
        if (!ft600_drv->WriteChipConfiguration(&oConfigurationData))
        {
            emit UpdLog("USB_device::FT600_Reconfig: Failure configuration FTDI");
            return false;
        }

        QThread::msleep(1000);
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_RECONF));

    if(FT600_Connect() == conn_err_init)
    {
        emit UpdLog("USB_device::FT600_Reconfig: Failure reconfig descriptor");
        return false;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_RECONF));

    SetFPGAPower(Pwr_ON);

    emit UpdLog("USB_device::FT600_Reconfig: Successful, FPGA power is ON");

    USB_busy = false;

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_RECONF));

    return true;
}

eProgInfo USB_device::FPGAProg(QString FileName, bool ui_data_width)       //  Загрузка прошивки FPGA
{
    FT_STATUS ftStatus;
    ULONG ulActualBytesWritten = 0;

    prog_value = 0;
    emit UpdProgBar(0);
    emit UpdLog("USB_device::FPGALoad: Start");

    //if(!FT600_Reconfig(false, ui_cyclone_LEs))
    //    return prog_err_reconf;

    QFile SrcFileChipZ64(FileName);

    if (!SrcFileChipZ64.open(QIODevice::ReadOnly))
    {
        emit UpdLog("USB_device::FPGALoad: '.rbf' file not openned");
        return prog_err_open_file;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_PROG));

/*******************************************************************************************************************************/

    ULONG uChipZ64Size = (ULONG)SrcFileChipZ64.size();

    char *databuf = new char[uChipZ64Size];
    ULONG *databuf32_load = new ULONG[uChipZ64Size * WORDS_4SEND_BYTE];
    USHORT *databuf16_load = new USHORT[uChipZ64Size * WORDS_4SEND_BYTE];

    ULONG DATA1_CLOCK0;
    ULONG DATA1_CLOCK1;
    ULONG DATA0_CLOCK0;
    ULONG DATA0_CLOCK1;

    SrcFileChipZ64.read(databuf, uChipZ64Size);
    SrcFileChipZ64.close();

    if(ui_data_width) // 16 bit FTDI
    {
        DATA1_CLOCK0 = 0x0000'0400;
        DATA1_CLOCK1 = 0x0000'0C00;
        DATA0_CLOCK0 = 0x0000'0000;
        DATA0_CLOCK1 = 0x0000'0800;
    }
    else
    {
        if(cyclone_LEs) // 32 bit FTDI, LEs 120 k
        {
            DATA1_CLOCK0 = 0x0000'8000; //бит 15
            DATA1_CLOCK1 = 0x0000'A000; //биты 15, 13
            DATA0_CLOCK0 = 0x0000'0000;
            DATA0_CLOCK1 = 0x0000'2000; //бит 13
        }
        else // 32 bit FTDI, LEs 80 k
        {
            DATA1_CLOCK0 = 0x4000'0000; //бит 30
            DATA1_CLOCK1 = 0xC000'0000; //биты 30, 31
            DATA0_CLOCK0 = 0x0000'0000;
            DATA0_CLOCK1 = 0x8000'0000; //бит 31
        }

    }

// nConfig
    ft600_drv->WriteGPIO(1, 0);
        Sleep(2);
    ft600_drv->WriteGPIO(1, 1);

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_PROG));

    quint32 k = 0;
    for(quint32 j = 0; j < uChipZ64Size; ++j)
    {
        for(quint32 i = 0; i < 8; ++i)
        {
            if((databuf[j] >> i) & 1)
            {
                if(ui_data_width)
                {
                    databuf16_load[k++] = (USHORT)DATA1_CLOCK0;  //data = 1 clock = 0
                    databuf16_load[k++] = (USHORT)DATA1_CLOCK1;  //data = 1 clock = 1
                }
                else
                {
                    databuf32_load[k++] = DATA1_CLOCK0;  //data = 1 clock = 0
                    databuf32_load[k++] = DATA1_CLOCK1;  //data = 1 clock = 1
                }
            }
            else
            {
                if(ui_data_width)
                {
                    databuf16_load[k++] = (USHORT)DATA0_CLOCK0;  //data = 0 clock = 0
                    databuf16_load[k++] = (USHORT)DATA0_CLOCK1;  //data = 0 clock = 1
                }
                else
                {
                    databuf32_load[k++] = DATA0_CLOCK0;  //data = 0 clock = 0
                    databuf32_load[k++] = DATA0_CLOCK1;  //data = 0 clock = 1
                }
            }
        }
    }

/*******************************************************************************************************************************/

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_PROG));

    if(ui_data_width)   ftStatus = ft600_drv->WritePipe(0x02, (UCHAR*)(databuf16_load), uChipZ64Size * WORDS_4SEND_BYTE * 2, &ulActualBytesWritten, NULL);
    else                ftStatus = ft600_drv->WritePipe(0x02, (UCHAR*)(databuf32_load), uChipZ64Size * WORDS_4SEND_BYTE * 4, &ulActualBytesWritten, NULL);

    if (ftStatus != FT_OK)
    {
        emit UpdLog("USB_device::FPGALoad: Function WritePipe failed (check Power)");

        delete[] databuf;
        delete[] databuf16_load;
        delete[] databuf32_load;

        return prog_err_wr;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/USB_PROG));

    delete[] databuf;
    delete[] databuf16_load;
    delete[] databuf32_load;

    emit UpdLog("USB_device::FPGALoad: Successful load '.rbf' in FPGA");

    return prog_ok;
}

bool USB_device::UsbWriteBuff(UCHAR* DataBuff, int BNum)
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
bool USB_device::UsbReadBuff(unsigned Readchars, ULONG* p_uLengthTransfered, UCHAR* ReadBuff) // Сколько считать, Сколько считать получилось, Куда считывать
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

// private:
    int USB_device::SetFPGAPower(eFPGAPOWER ePwr)                   //  Управление питанием FPGA.
    {
        if (ePwr == Pwr_ON) ft600_drv->WriteGPIO(2, 3);// вкл питание
        else ft600_drv->WriteGPIO(2, 0);// выкл питание

        Sleep(100);
        return 0;
    }

// setter/getter:
    bool USB_device::getUSB_busy() const
    {
        return USB_busy;
    }
    bool USB_device::getCyclone_LEs() const
    {
        return cyclone_LEs;
    }
