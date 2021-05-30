#include "FPGA_device.h"

FPGA_device::FPGA_device()
{
    usb_device = new USB_device();

    connect(usb_device, SIGNAL(UpdLog(QString)), this, SLOT(UpdLogBridge(QString)));
    connect(usb_device, SIGNAL(UpdProgBar(int)), this, SLOT(UpdProgBarBridge(int)));

    connect(usb_device, SIGNAL(UpdFTDIDesc(QString)), this, SLOT(UpdFTDIDescBridge(QString)));

    connect(usb_device, SIGNAL(GetCycloneLEs()), this, SLOT(GetCycloneLEsBridge()));
    connect(usb_device, SIGNAL(SetCycloneLEs(bool)), this, SLOT(SetCycloneLEsBridge(bool)));

    connect(usb_device, SIGNAL(ShowMsg(QString,QString)), this, SLOT(ShowMsgBridge(QString,QString)));
}

FPGA_device::~FPGA_device()
{
   if (usb_device)
   {
       delete usb_device;
       usb_device = nullptr;
   }
}

// Начальная инициализация устройства
bool FPGA_device::Initialize(QString RbfFileName)
{
    QString rbf_path;

    emit UpdLog("FPGA_device::Initialize: Start");

    if (!usb_device->FT600_Connect())
        return false;

    if(emit GetChBoxDefPath())
    {
        if(usb_device->getCyclone_LEs()) // true - 120k LEs version, false - 80k
        {
            rbf_path = RBF120_PATH;
            emit UpdRbfPath(RBF120_PATH);
        }
        else
        {
            rbf_path = RBF080_PATH;
            emit UpdRbfPath(RBF080_PATH);
        }
    }
    else rbf_path = RbfFileName;

    if (!usb_device->FPGALoad(rbf_path))
       return false;

// read firmware data, ver:
    bool read_ok;

    ULONG RdCnt;
    ULONG out_data[2];

    UINT ver_high, ver_low;
    UINT day, mounth, year;

    if(emit GetDataWidth()) // true - 16 bit, false - 32 bit (default)
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
                ", " + QString::number(out_data[0]));

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
        return true;
    }
    else
    {
        emit UpdVerFPGA("Error");
        emit UpdDateFPGA("Error");

        emit UpdLog("FPGA_device::Initialize: Failure to read FPGA services regs");
        return false;
    }

    emit UpdLog("FPGA_device::Initialize: Successful read FPGA services regs");
    return true;
}

bool FPGA_device::StartTest(QString HexFileName)
{
    emit UpdLog("FPGA_device::StartTest: Start");

    //QTimer wait_usb;
    int time_usb = 0;

    QFile hex_file(HexFileName);

    if (!hex_file.open(QIODevice::ReadOnly))
    {
       emit ShowMsg("Wrong Path", "File " + HexFileName + " not opened");
       emit UpdLog("FPGA_device::StartTest: test data '.hex' file not openned");
       return false;
    }

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
        return false;
    }

    hex_file.reset();

    ULONG   Wr_Data32[buf_size];
    USHORT  Wr_Data16[buf_size];

    UCHAR data_width;

    bool choose_width = emit GetDataWidth();

    if(choose_width) data_width = 4; // true - 16 bit, false - 32 bit (default)
    else data_width = 8;

    for(UINT i = 0; i < buf_size; ++i)
    {
        if(choose_width) in_str.readLine(data_width); // empty 4 MSB hex symbols

        line = in_str.readLine(data_width);
        in_str.readLine();

        if(choose_width) Wr_Data16[i] = line.toUShort(0, 16);
        else Wr_Data32[i] = line.toULong(0, 16);
    }

    hex_file.close();

    emit UpdLog("FPGA_device::StartTest: Written array:\n\n\tWritten array:\n");

    line = "";
    for(UINT i = 0; i < BUF_COL_SHOW; ++i)
    {
        for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
        {
            if(choose_width) line.append(QString::number(Wr_Data16[j + i*BUF_ROW_SHOW], 16).rightJustified(data_width, '0').toUpper() + "\t");
            else line.append(QString::number(Wr_Data32[j + i*BUF_ROW_SHOW], 16).rightJustified(data_width, '0').toUpper() + "\t");
        }

        emit UpdLog(line);
        line = "";
    }

    emit UpdLog("\n...\t...\t...\t...\n");

    UINT buf_bias = buf_size - MIN_BUF_SIZE/2;

    line = "";
    for(UINT i = 0; i < BUF_COL_SHOW; ++i)
    {
        for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
        {
            if(choose_width) line.append(QString::number(Wr_Data16[j + i*BUF_ROW_SHOW + buf_bias], 16).rightJustified(data_width, '0').toUpper() + "\t");
            else line.append(QString::number(Wr_Data32[j + i*BUF_ROW_SHOW + buf_bias], 16).rightJustified(data_width, '0').toUpper() + "\t");
        }

        emit UpdLog(line);
        line = "";
    }
    emit UpdLog("");

    emit UpdLog("FPGA_device::StartTest: Wait USB...");
    while(usb_device->getUSB_busy()) // WARNING !!!!! required add timer for avoid freeze
    {
        //wait_usb.start(100); // ms
        QThread::msleep(1000);
        emit UpdLog("FPGA_device::StartTest: ...");

        if(time_usb > 10)
        {
            emit UpdLog("FPGA_device::StartTest: Failed! USB is busy");
            return false;
        }

        ++time_usb;
    }

    bool wr_ok;

    if(choose_width) wr_ok = usb_device->UsbWrite((UCHAR*)Wr_Data16, 2*buf_size);
    else wr_ok = usb_device->UsbWrite((UCHAR*)Wr_Data32, 4*buf_size);

    if(!wr_ok)
    {
       emit UpdLog("FPGA_device::StartTest: Failed write data in FPGA");
       return false;
    }

    emit UpdLog("FPGA_device::StartTest: Success write data in FPGA");

    bool read_ok;

    ULONG Data_pnt[buf_size];
    ULONG RdCnt;

    QThread::msleep(100);
    //wait_usb.start(1000); // ms

    if(choose_width) read_ok = usb_device->UsbReadBuff(2*buf_size, &RdCnt, (UCHAR*)(Data_pnt));
    else read_ok = usb_device->UsbReadBuff(4*buf_size, &RdCnt, (UCHAR*)(Data_pnt));

    if(!read_ok)
    {
        emit UpdLog("FPGA_device::StartTest: Falure to read test array from FPGA");
        return false;
    }

    emit UpdLog("FPGA_device::StartTest: Readed array:\n\n\tReaded array:\n");

    line = "";
    for(UINT i = 0; i < BUF_COL_SHOW; ++i)
    {
        for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
            line.append(QString::number(Data_pnt[j + i*BUF_ROW_SHOW], 16).rightJustified(data_width, '0').toUpper() + "\t");

        emit UpdLog(line);
        line = "";
    }

    emit UpdLog("\n...\t...\t...\t...\n");

    buf_bias = buf_size - MIN_BUF_SIZE/2;

    line = "";
    for(UINT i = 0; i < BUF_COL_SHOW; ++i)
    {
        for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
            line.append(QString::number(Data_pnt[j + i*BUF_ROW_SHOW + buf_bias], 16).rightJustified(data_width, '0').toUpper() + "\t");

        emit UpdLog(line);
        line = "";
    }

    return true;
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

bool FPGA_device::GetCycloneLEsBridge()
{
    return emit GetCycloneLEs();
}

void FPGA_device::SetCycloneLEsBridge(bool cyclone_LEs)
{
    emit SetCycloneLEs(cyclone_LEs);
}

void FPGA_device::ShowMsgBridge(QString title, QString msg)
{
    emit ShowMsg(title, msg);
}
