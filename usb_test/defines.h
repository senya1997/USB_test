#ifndef DEFINES_H
#define DEFINES_H

#include <QString>

using USHORT    = unsigned short;
using UCHAR     = unsigned char;
using UINT      = unsigned int;
using ULONG     = unsigned long;

// main frame size:
    const int MAIN_H = 285;
    const int MAIN_WL = 319;
    const int MAIN_WH = 690;

// default paths:
    const QString RBF_PATH = "../verilog/output_files/usb_test.rbf";
    const QString HEX_PATH = "../verilog/test_data.hex";

//Устройство, которое мы ищем (должно быть зашито в FTDI 600)
    static const char ZDOPP_DEVICE_NAME[]			= "v1 10CL120";
    static const char* g_chSerialNumber             = "000000000001";

    //static const char ZDOPP_DEVICE_NAME[]			= "ZDopp USB3.0 v1";
    //static const char* g_chSerialNumber             = "000000000001";

    static const char VENDOR_NAME[]					= "JS Zaslon";

// number of step progress bar in appropriate method:
    const int PROG_CONNECT = 3;
    const int PROG_LOAD = 3;

// for show part of recieved/transmit data array:
    const UINT BUF_ROW_SHOW = 8;
    const UINT BUF_COL_SHOW = 4;

    const UINT MIN_BUF_SIZE = 2 * BUF_ROW_SHOW * BUF_COL_SHOW;

enum eFPGAPOWER {
    Pwr_OFF,
    Pwr_ON
};

#endif
