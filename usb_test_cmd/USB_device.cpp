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

    char *desc = nullptr;

// check FTDI connect:
    ft600_drv->Cleanup();
    ft600_drv->GetDevicesInfoList(&ptrInfo);

    while(ptrInfo->Description[desc_size] != 0x00) ++desc_size; // calc descriptor string size in common info about connected FTDI
    ++desc_size;

    desc = new char[desc_size];

    for(int i = 0; i < desc_size; ++i)
        desc[i] = ptrInfo->Description[i];

    if(!ft600_drv->Initialize(EOPEN_BY_DESC, (PVOID)desc, (PVOID)g_chSerialNumber))
    {
        delete[] desc;
        desc = nullptr;

        return conn_err_init;
    }

// check descriptor:
    if(strcmp(desc, DEVICE_NAME120) == 0)
        cyclone_LEs = true;
    else if(strcmp(desc, DEVICE_NAME080) == 0)
        cyclone_LEs = false;
    else
    {
        delete[] desc;
        desc = nullptr;

        return conn_wrong_desc;
    }

    delete[] desc;
    desc = nullptr;

    return conn_ok;
}
bool USB_device::FT600_Disconnect(void)
{
    if (ft600_drv)
    {
        SetFPGAPower(Pwr_OFF);
        delete ft600_drv;
        ft600_drv = nullptr;

        return true;
    }
    else
        return false;
}
bool USB_device::FT600_Reconfig(bool force_reconf, bool ui_cyclone_LEs)
{
    FT_60XCONFIGURATION oConfigurationData;
    eConnInfo ftdi_connect;

    bool isCorrected = false;
    bool in_force_reconf;

    prog_value = 0;

    ftdi_connect = FT600_Connect();

    if(ftdi_connect == conn_err_init)
        return false;
    else if((ftdi_connect == conn_wrong_desc) | force_reconf)
        in_force_reconf = true;
    else
        in_force_reconf = false;

    ++prog_value;

// read configuration:
    if (!ft600_drv->ReadChipConfiguration(&oConfigurationData))
        return false;

    ++prog_value;

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

//Пишем, если корректировалась
    if (isCorrected)
    {
        if (!ft600_drv->WriteChipConfiguration(&oConfigurationData))
            return false;

        this_thread::sleep_for(1s);
    }

    ++prog_value;

    if(FT600_Connect() == conn_err_init)
        return false;

    SetFPGAPower(Pwr_ON);

    ++prog_value;

    return true;
}

eProgInfo USB_device::FPGAProg(string FileName, bool ui_data_width)       //  Загрузка прошивки FPGA
{
    FT_STATUS ftStatus;
    ulong ulActualBytesWritten = 0;

    ifstream rbf_file(FileName);
    ulong rbf_size;

    char *databuf = nullptr;
    ulong *databuf32_load = nullptr;
    ushort *databuf16_load = nullptr;

    ulong DATA1_CLOCK0;
    ulong DATA1_CLOCK1;
    ulong DATA0_CLOCK0;
    ulong DATA0_CLOCK1;

    uint32_t k = 0;

    prog_value = 0;

    //if(!FT600_Reconfig(false, ui_cyclone_LEs))
    //    return prog_err_reconf;

    if (!rbf_file.is_open())
        return prog_err_open_file;

    ++prog_value;

/*******************************************************************************************************************************/

    rbf_file.seekg(0, ios::end); // 1 char = 1 byte => go to the end of file and get position of cursor
        rbf_size = rbf_file.tellg(); // in byte
    rbf_file.seekg(0, ios::beg); // return position in begin

    databuf = new char[rbf_size];

    if(ui_data_width)   databuf16_load = new ushort[rbf_size * WORDS_4SEND_BYTE];
    else                databuf32_load = new ulong[rbf_size * WORDS_4SEND_BYTE];

    rbf_file.read(databuf, rbf_size);
    rbf_file.close();

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
        this_thread::sleep_for(2s);
    ft600_drv->WriteGPIO(1, 1);

    ++prog_value;

    for(uint32_t j = 0; j < rbf_size; ++j)
    {
        for(uint32_t i = 0; i < 8; ++i)
        {
            if((databuf[j] >> i) & 1)
            {
                if(ui_data_width)
                {
                    databuf16_load[k++] = (ushort)DATA1_CLOCK0;  //data = 1 clock = 0
                    databuf16_load[k++] = (ushort)DATA1_CLOCK1;  //data = 1 clock = 1
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
                    databuf16_load[k++] = (ushort)DATA0_CLOCK0;  //data = 0 clock = 0
                    databuf16_load[k++] = (ushort)DATA0_CLOCK1;  //data = 0 clock = 1
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

    if(ui_data_width)   ftStatus = ft600_drv->WritePipe(0x02, (UCHAR*)(databuf16_load), rbf_size * WORDS_4SEND_BYTE * 2, &ulActualBytesWritten, NULL);
    else                ftStatus = ft600_drv->WritePipe(0x02, (UCHAR*)(databuf32_load), rbf_size * WORDS_4SEND_BYTE * 4, &ulActualBytesWritten, NULL);

    delete[] databuf;
    delete[] databuf16_load;
    delete[] databuf32_load;

    databuf = nullptr;
    databuf16_load = nullptr;
    databuf32_load = nullptr;

    if (ftStatus != FT_OK)
        return prog_err_wr;

    ++prog_value;

    return prog_ok;
}

bool USB_device::UsbWriteBuff(UCHAR* DataBuff, int BNum)
{
    FT_STATUS ftStatus = FT_OK;             //	Статус операции записи

    ulong ulActualBytesWritten = 0;			//	Колличество записанных байтов

    ftStatus = ft600_drv->WritePipe(0x02, (PUCHAR)DataBuff, BNum, &ulActualBytesWritten, NULL);	//	Загрузка массива данных в ПЛИС

    if (ftStatus == FT_OK)
        return true;
    else
        return false;
}
bool USB_device::UsbReadBuff(unsigned Readchars, ulong* p_uLengthTransfered, UCHAR* ReadBuff) // Сколько считать, Сколько считать получилось, Куда считывать
{
    FT_STATUS ftStatus = FT_OK;						//	Статус операции записи

    //	If lpOverlapped is NULL, FT_ReadPipe operates synchronously, that is, it returns only when
    //	the transfer has been completed.

    ftStatus = ft600_drv->ReadPipe(0x82, ReadBuff, Readchars, p_uLengthTransfered, NULL);

    if (ftStatus == FT_OK)
        return true;
    else
        return false;
}

// private:
    int USB_device::SetFPGAPower(eFPGAPOWER ePwr)                   //  Управление питанием FPGA.
    {
        if (ePwr == Pwr_ON) ft600_drv->WriteGPIO(2, 3);// вкл питание
        else ft600_drv->WriteGPIO(2, 0);// выкл питание

        this_thread::sleep_for(100ms);
        return 0;
    }

// setter/getter:
    bool USB_device::getCyclone_LEs() const
    {
        return cyclone_LEs;
    }
