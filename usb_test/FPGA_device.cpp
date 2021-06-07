#include "FPGA_device.h"

FPGA_device::FPGA_device()
{
    usb_device = new USB_device();

    connect(usb_device, SIGNAL(UpdLog(QString)), this, SLOT(UpdLogBridge(QString)));
    connect(usb_device, SIGNAL(UpdProgBar(int)), this, SLOT(UpdProgBarBridge(int)));

    connect(usb_device, SIGNAL(UpdFTDIDesc(QString)), this, SLOT(UpdFTDIDescBridge(QString)));

    connect(usb_device, SIGNAL(SetCycloneLEs(bool)), this, SLOT(SetCycloneLEsBridge(bool)));
}

FPGA_device::~FPGA_device()
{
   if (usb_device)
   {
       usb_device->FT600_Disconnect();

       delete usb_device;
       usb_device = nullptr;
   }
}

// Начальная инициализация устройства
eInitInfo FPGA_device::Initialize(bool force_reconf, bool ui_cyclone_LEs)
{
    emit UpdLog("FPGA_device::Initialize: Start");

    if(force_reconf)
    {
        if(!usb_device->FT600_Reconfig(force_reconf, ui_cyclone_LEs))
            return init_err_reconf;
    }
    else
    {
        if(usb_device->FT600_Connect() == conn_err_init)
            return init_failed;
        else if(usb_device->FT600_Connect() == conn_wrong_desc)
            return init_wrong_desc;
    }

    emit UpdLog("FPGA_device::Initialize: Succesful FTDI connected!");
    return init_ok;
}

eProgInfo FPGA_device::Program(QString RbfFileName, bool def_path, bool ui_data_width, bool ui_cyclone_LEs)
{
    QString rbf_path;

    emit UpdLog("FPGA_device::Initialize: Start");

    if(!usb_device->FT600_Reconfig(false, ui_cyclone_LEs)) // not good, better if resolve conflict between RBF path and call reconfig for read descriptor
        return prog_err_reconf;

    if(def_path)
    {
        if(usb_device->getCyclone_LEs()) // true - 120k LEs version, false - 80k
        {
            if(ui_data_width)
                emit SetDataWidth(false);

            rbf_path = RBF120_32BIT_PATH;
            emit UpdRbfPath(RBF120_32BIT_PATH);
        }
        else
        {
            if(ui_data_width)
            {
                rbf_path = RBF080_16BIT_PATH;
                emit UpdRbfPath(RBF080_16BIT_PATH);
            }
            else
            {
                rbf_path = RBF080_32BIT_PATH;
                emit UpdRbfPath(RBF080_32BIT_PATH);
            }
        }
    }
    else rbf_path = RbfFileName;

    eProgInfo prog_info = usb_device->FPGAProg(rbf_path, ui_data_width);

    if (prog_info != prog_ok)
       return prog_info;

// read firmware data, ver:
    bool read_ok;

    ULONG RdCnt;
    ULONG out_data[2];

    UINT ver_high, ver_low;
    UINT day, mounth, year;

    if(ui_data_width) // true - 16 bit, false - 32 bit (default)
    {
        USHORT Data_pnt[2];
        read_ok = usb_device->UsbReadBuff(4, &RdCnt, (UCHAR*)(Data_pnt));

        out_data[0] = (ULONG)(Data_pnt[0]);
        out_data[1] = (ULONG)(Data_pnt[1]);
    }
    else
    {
        ULONG Data_pnt[2];
        read_ok = usb_device->UsbReadBuff(8, &RdCnt, (UCHAR*)(Data_pnt));

        out_data[0] = Data_pnt[0];
        out_data[1] = Data_pnt[1];
    }

    emit UpdLog("     RdCnt: " + QString::number(RdCnt) +
                "; Data_pnt: " + QString::number(out_data[0]) +
                ", " + QString::number(out_data[1]));

    ver_high = out_data[0]/1000;
    ver_low = out_data[0]%1000;

    mounth = out_data[1]/10'000;
    year = out_data[1]%100;
    day = (out_data[1]%10'000)/100;

    if(read_ok) // mb required delay before read
    {
        emit UpdVerFPGA(QString::number(ver_high, 10).rightJustified(2, '0') + "." +
                        QString::number(ver_low, 10).rightJustified(3, '0'));

        emit UpdDateFPGA(QString::number(mounth, 10).rightJustified(2, '0') + "." +
                         QString::number(day, 10).rightJustified(2, '0') + "." +
                         QString::number(year, 10).rightJustified(2, '0'));

        emit UpdLog("FPGA_device::Initialize: Successful read FPGA services regs");
        return prog_ok;
    }
    else
    {
        emit UpdVerFPGA("Error");
        emit UpdDateFPGA("Error");

        emit UpdLog("FPGA_device::Initialize: Failure to read FPGA services regs");
        return prog_err_rd;
    }
}

eTestInfo FPGA_device::StartTest(QString HexFileName, bool ui_data_width)
{
    UINT prog_value = 0;
    emit UpdProgBar(0);
    emit UpdLog("FPGA_device::StartTest: Start");

    //QTimer wait_usb;
    int time_usb = 0;

    QFile hex_file(HexFileName);

    if (!hex_file.open(QIODevice::ReadOnly))
    {
       emit UpdLog("FPGA_device::StartTest: test data '.hex' file not openned");
       return test_err_open_file;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));

// calc size of buf test data:
    QString line;
    QTextStream in_str(&hex_file);
    UINT buf_size = 0;

    while(!in_str.atEnd())
    {
        in_str.readLine();
        buf_size++;
    }

    if(buf_size < MIN_BUF_SIZE)
    {
        emit UpdLog("FPGA_device::StartTest: Error: test array size is smaller then " + QString::number(MIN_BUF_SIZE));
        hex_file.close();
        return test_err_arr_size;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));

    hex_file.reset();

// read HEX, get test data:
    ULONG   Wr_Data32[buf_size];
    USHORT  Wr_Data16[buf_size];

    UCHAR data_width;

    if(ui_data_width) data_width = 4; // true - 16 bit, false - 32 bit (default)
    else data_width = 8;

    for(UINT i = 0; i < buf_size; ++i)
    {
        if(ui_data_width) in_str.readLine(data_width); // empty 4 MSB hex symbols

        line = in_str.readLine(data_width);
        in_str.readLine();

        if(ui_data_width) Wr_Data16[i] = line.toUShort(0, 16);
        else Wr_Data32[i] = line.toULong(0, 16);
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));

    hex_file.close();

// wait USB:
    emit UpdLog("FPGA_device::StartTest: Wait USB...");
    while(usb_device->getUSB_busy()) // WARNING !!!!! required add timer for avoid freeze
    {
        //wait_usb.start(100); // ms
        QThread::msleep(500);
        emit UpdLog("FPGA_device::StartTest: ...");

        if(time_usb > 10)
        {
            emit UpdLog("FPGA_device::StartTest: Failed! USB is busy");
            return test_err_usb_busy;
        }

        ++time_usb;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));

// write/read:
    bool wr_ok;
    bool read_ok;
    bool test_passed = true;

    ULONG Data32_pnt[buf_size];
    USHORT Data16_pnt[buf_size];

    ULONG RdCnt;

    if(ui_data_width) wr_ok = usb_device->UsbWriteBuff((UCHAR*)Wr_Data16, 2*buf_size);
    else wr_ok = usb_device->UsbWriteBuff((UCHAR*)Wr_Data32, 4*buf_size);

    if(!wr_ok)
    {
       emit UpdLog("FPGA_device::StartTest: Failed write data in FPGA");
       return test_err_wr;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));
    emit UpdLog("FPGA_device::StartTest: Success write data in FPGA");

    QThread::msleep(100);
    //wait_usb.start(1000); // ms

    if(ui_data_width) read_ok = usb_device->UsbReadBuff(2*buf_size, &RdCnt, (UCHAR*)(Data16_pnt));
    else read_ok = usb_device->UsbReadBuff(4*buf_size, &RdCnt, (UCHAR*)(Data32_pnt));

    if(!read_ok)
    {
        emit UpdLog("FPGA_device::StartTest: Falure to read test array from FPGA");
        return test_err_rd;
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));

// check data:
    emit UpdLog("");
    for(UINT i = 0; i < buf_size; ++i)
    {
        if(ui_data_width)
        {
            if(Wr_Data16[i] != Data16_pnt[i])
            {
                emit UpdLog("\t" + QString::number(i) + ": " +
                            QString::number(Wr_Data16[i], 16).rightJustified(data_width, '0').toUpper() + " - " +
                            QString::number(Data16_pnt[i], 16).rightJustified(data_width, '0').toUpper());
                test_passed = false;
            }
        }
        else
        {
            if(Wr_Data32[i] != Data32_pnt[i])
            {
                emit UpdLog("\t" + QString::number(i) + ": " +
                            QString::number(Wr_Data32[i], 16).rightJustified(data_width, '0').toUpper() + " - " +
                            QString::number(Data32_pnt[i], 16).rightJustified(data_width, '0').toUpper());
                test_passed = false;
            }
        }
    }

    ++prog_value;
    emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));

// show result:
    if(test_passed)
    {
        emit UpdLog("FPGA_device::StartTest: Readed array:\n\tReaded array (HEX):\n");

        line = "";
        for(UINT i = 0; i < BUF_COL_SHOW; ++i)
        {
            if(ui_data_width)
            {
                for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
                    line.append(QString::number(Data16_pnt[j + i*BUF_ROW_SHOW], 16).rightJustified(data_width, '0').toUpper() + "\t");
            }
            else
            {
                for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
                    line.append(QString::number(Data32_pnt[j + i*BUF_ROW_SHOW], 16).rightJustified(data_width, '0').toUpper() + "\t");
            }

            emit UpdLog(line);
            line = "";
        }

        emit UpdLog("\n...\t...\t...\t...\n");

        UINT buf_bias = buf_size - MIN_BUF_SIZE/2;

        line = "";
        for(UINT i = 0; i < BUF_COL_SHOW; ++i)
        {
            if(ui_data_width)
            {
                for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
                    line.append(QString::number(Data16_pnt[j + i*BUF_ROW_SHOW + buf_bias], 16).rightJustified(data_width, '0').toUpper() + "\t");
            }
            else
            {
                for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
                    line.append(QString::number(Data32_pnt[j + i*BUF_ROW_SHOW + buf_bias], 16).rightJustified(data_width, '0').toUpper() + "\t");
            }

            emit UpdLog(line);
            line = "";
        }

        ++prog_value;
        emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));

        return test_ok;
    }
    else
    {
        ++prog_value;
        emit UpdProgBar((int)((float)prog_value*100/FPGA_TEST));
        emit UpdLog("\tnum: Transmit - Recieve (HEX)\n");
        emit UpdLog("FPGA_device::StartTest: Test failed!");

        return test_failed;
    }
}

void FPGA_device::UpdLogBridge(QString log_str)
{
    emit UpdLog(log_str);
}

void FPGA_device::UpdProgBarBridge(int prog_value)
{
    emit UpdProgBar(prog_value);
}

void FPGA_device::UpdFTDIDescBridge(QString desc)
{
    emit UpdFTDIDesc(desc);
}

void FPGA_device::SetCycloneLEsBridge(bool cyclone_LEs)
{
    emit SetCycloneLEs(cyclone_LEs);
}
