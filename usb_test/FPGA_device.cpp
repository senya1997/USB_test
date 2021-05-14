#include "FPGA_device.h"

FPGA_device::FPGA_device()
{
    usb_device = new USB_device();

    connect(usb_device, SIGNAL(UpdLog(QString)), this, SLOT(UpdLogBridge(QString)));
    connect(usb_device, SIGNAL(UpdProgBar(int)), this, SLOT(UpdProgBarBridge(int)));

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
    emit UpdLog("FPGA_device::Initialize: Start");

    if (!usb_device->FT600_Connect())
       return false;

    if (!usb_device->FPGALoad(RbfFileName, &CurrFPGAVer))
       return false;

// read firmware settings:
    ULONG Data_pnt[2];
    ULONG RdCnt;                                     //	Текущее колличество слов, считанное из ПЛИС

    if(usb_device->UsbReadBuff(8, &RdCnt, (UCHAR*)(Data_pnt))) // mb required delay before read
    {
        emit UpdVerFPGA(QString::number(Data_pnt[0]));
        emit UpdDateFPGA(QString::number(Data_pnt[1]));

        emit UpdLog("FPGA_device::Initialize: Successful read FPGA services regs");
        return true;
    }
    else
    {
        emit UpdVerFPGA("Error");
        emit UpdDateFPGA("Error");

        emit UpdLog("     RdCnt: " + QString::number(RdCnt) +
                    "; Data_pnt: " + QString::number(Data_pnt[0]) +
                    ", " + QString::number(Data_pnt[0]));
        emit UpdLog("FPGA_device::Initialize: Failure to read FPGA services regs");
        return false;
    }

    emit UpdLog("FPGA_device::Initialize: Successful read FPGA services regs");
    return true;
}

bool FPGA_device::StartTest(QString HexFileName)
{
    emit UpdLog("FPGA_device::StartTest: Start");

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

    UINT Wr_Data[buf_size];

    UCHAR data_width;

    if(emit GetDataWidth()) data_width = 4; // true - 16 bit, false - 32 bit (default)
    else data_width = 8;

    for(UINT i = 0; i < buf_size; ++i)
    {
        line = in_str.readLine(data_width);

        in_str.readLine();
        Wr_Data[i] = line.toUInt(0, 16);
    }

    hex_file.close();

    emit UpdLog("FPGA_device::StartTest: Written array:");

    emit UpdLog("\n\tWritten array:\n");

    line = "";
    for(UINT i = 0; i < BUF_COL_SHOW; ++i)
    {
        for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
            line.append(QString::number(Wr_Data[j + i*BUF_ROW_SHOW], 16).rightJustified(data_width, '0').toUpper() + "\t");

        emit UpdLog(line);
        line = "";
    }

    emit UpdLog("\n...\t...\t...\t...\n");

    UINT buf_bias = buf_size - MIN_BUF_SIZE/2;

    line = "";
    for(UINT i = 0; i < BUF_COL_SHOW; ++i)
    {
        for(UINT j = 0; j < BUF_ROW_SHOW; ++j)
            line.append(QString::number(Wr_Data[j + i*BUF_ROW_SHOW + buf_bias], 16).rightJustified(data_width, '0').toUpper() + "\t");

        emit UpdLog(line);
        line = "";
    }
/*
    while(usb_device->getUSB_busy()); // WARNING !!!!! required add timer for avoid freeze

    if (!usb_device->UsbWrite(&Wr_Data[0], buf_size))
    {
       emit UpdLog("FPGA_device::StartTest: Failed write data in FPGA");
       return false;
    }

    emit UpdLog("FPGA_device::StartTest: Success write data in FPGA");
*/


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

void FPGA_device::ShowMsgBridge(QString title, QString msg)
{
    emit ShowMsg(title, msg);
}
