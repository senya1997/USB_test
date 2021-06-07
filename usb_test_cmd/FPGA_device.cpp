#include "FPGA_device.h"

FPGA_device::FPGA_device()
{
    usb_device = new USB_device();
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

eInitInfo FPGA_device::Initialize(bool force_reconf, bool ui_cyclone_LEs)
{
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

    return init_ok;
}

eProgInfo FPGA_device::Program(bool ui_data_width, ulong *ver, ulong *date)
{
    string rbf_path;
    eProgInfo prog_info;

    bool read_ok;
    ulong RdCnt;

    //if(!usb_device->FT600_Reconfig(false, ui_cyclone_LEs)) // not good, better if resolve conflict between RBF path and call reconfig for read descriptor
    //    return prog_err_reconf;

    if(usb_device->getCyclone_LEs()) // true - 120k LEs version, false - 80k
        rbf_path = RBF120_32BIT_PATH;
    else
    {
        if(ui_data_width)
            rbf_path = RBF080_16BIT_PATH;
        else
            rbf_path = RBF080_32BIT_PATH;
    }

    prog_info = usb_device->FPGAProg(rbf_path, ui_data_width);

    if (prog_info != prog_ok)
       return prog_info;

// read firmware data, ver:
    if(ui_data_width) // true - 16 bit, false - 32 bit (default)
    {
        ushort Data_pnt[2];
        read_ok = usb_device->UsbReadBuff(4, &RdCnt, (uchar*)(Data_pnt));

        *ver = (ulong)(Data_pnt[0]);
        *date = (ulong)(Data_pnt[1]);
    }
    else
    {
        ulong Data_pnt[2];
        read_ok = usb_device->UsbReadBuff(8, &RdCnt, (uchar*)(Data_pnt));

        *ver = Data_pnt[0];
        *date = Data_pnt[1];
    }

    if(read_ok) // mb required delay before read
        return prog_ok;
    else
        return prog_err_rd;
}

eTestInfo FPGA_device::StartTest(string HexFileName, bool ui_data_width, ulong *num_err, ulong data_exp[], ulong data_err[])
{
    uint16_t prog_value = 0;

// read HEX, get test data:
    ifstream hex_file(HexFileName);

    uint16_t buf_size = 0;
    string str_buf;

    ushort  *Wr_Data16 = nullptr;
    ulong   *Wr_Data32 = nullptr;

// write/read:
    bool wr_ok;
    bool read_ok;
    bool test_passed = true;

    ushort  *Data16_pnt = nullptr;
    ulong   *Data32_pnt = nullptr;

    ulong RdCnt;

// read HEX calc size of buf test data:
    if (!hex_file.is_open())
        return test_err_open_file;

    ++prog_value;

    while(!hex_file.eof())
    {
        getline(hex_file, str_buf);
        buf_size++;
    }

    if(buf_size < MIN_BUF_SIZE)
    {
        hex_file.close();
        return test_err_arr_size;
    }

    ++prog_value;

    hex_file.seekg(0, ios::beg);

// read HEX, get test data:
    if(ui_data_width)
    {
        Wr_Data16 = new ushort[buf_size];
        Data16_pnt = new ushort[buf_size];
    }
    else
    {
        Wr_Data32 = new ulong[buf_size];
        Data32_pnt = new ulong[buf_size];
    }

    for(uint16_t i = 0; i < buf_size; ++i)
    {
        getline(hex_file, str_buf);

        if(ui_data_width)
        {
            str_buf = str_buf.substr(HEX16_LENGTH, HEX32_LENGTH - 1); // empty 4 MSB hex symbols
            Wr_Data16[i] = (ushort)strtol(str_buf.c_str(), NULL, 16);
        }
        else
            Wr_Data32[i] = strtoul(str_buf.c_str(), NULL, 16);
    }

    ++prog_value;

    hex_file.close();

// write/read:
    if(ui_data_width) wr_ok = usb_device->UsbWriteBuff((UCHAR*)Wr_Data16, 2*buf_size);
    else wr_ok = usb_device->UsbWriteBuff((UCHAR*)Wr_Data32, 4*buf_size);

    if(!wr_ok)
    {
        delete[] Wr_Data16;
        delete[] Wr_Data32;

        delete[] Data16_pnt;
        delete[] Data32_pnt;

        Wr_Data16 = nullptr;
        Wr_Data32 = nullptr;

        Data16_pnt = nullptr;
        Data32_pnt = nullptr;

        return test_err_wr;
    }

    ++prog_value;

    this_thread::sleep_for(100ms); // mb required large delay

    if(ui_data_width) read_ok = usb_device->UsbReadBuff(2*buf_size, &RdCnt, (UCHAR*)(Data16_pnt));
    else read_ok = usb_device->UsbReadBuff(4*buf_size, &RdCnt, (UCHAR*)(Data32_pnt));

    if(!read_ok)
    {
        delete[] Wr_Data16;
        delete[] Wr_Data32;

        delete[] Data16_pnt;
        delete[] Data32_pnt;

        Wr_Data16 = nullptr;
        Wr_Data32 = nullptr;

        Data16_pnt = nullptr;
        Data32_pnt = nullptr;

        return test_err_rd;
    }

    ++prog_value;

// check data:
    *num_err = 0;

    for(uint16_t i = 0; i < buf_size; ++i)
    {
        if(ui_data_width)
        {
            if(Wr_Data16[i] != Data16_pnt[i])
            {
                *num_err = *num_err + 1;
                data_exp[i] = Wr_Data16[i];
                data_err[i] = Data16_pnt[i];

                test_passed = false;
            }
        }
        else
        {
            if(Wr_Data32[i] != Data32_pnt[i])
            {
                *num_err = *num_err + 1;
                data_exp[i] = Wr_Data32[i];
                data_err[i] = Data32_pnt[i];

                test_passed = false;
            }
        }
    }

    Wr_Data16 = nullptr;
    Wr_Data32 = nullptr;

    Data16_pnt = nullptr;
    Data32_pnt = nullptr;

    delete[] Data16_pnt;
    delete[] Data32_pnt;

    delete[] Wr_Data16;
    delete[] Wr_Data32;

    ++prog_value;

// show result:
    if(test_passed)
        return test_ok;
    else
        return test_failed;
}
