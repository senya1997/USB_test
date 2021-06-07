#ifndef DEFINES_H
#define DEFINES_H

#include <QString>

using USHORT    = unsigned short;
using UCHAR     = unsigned char;
using UINT      = unsigned int;
using ULONG     = unsigned long;

enum eConnInfo{
    conn_ok,
    conn_err_init,
    conn_wrong_desc
};

enum eInitInfo{
    init_ok,
    init_err_reconf,
    init_wrong_desc,
    init_failed,
};

enum eProgInfo{
    prog_ok,
    prog_err_open_file,
    prog_err_reconf,
    prog_err_wr,
    prog_err_rd
};

enum eTestInfo{
    test_ok,
    test_err_open_file,
    test_err_arr_size,
    test_err_usb_busy,
    test_err_wr,
    test_err_rd,
    test_failed
};

// main frame size:
    const UINT MAIN_H = 345;
    const UINT MAIN_WL = 318;
    const UINT MAIN_WH = 691;

// default paths:
    const QString RBF080_16BIT_PATH = "../verilog/output_files/usb_test080_16bit.rbf";
    const QString RBF080_32BIT_PATH = "../verilog/output_files/usb_test080_32bit.rbf";

    //const QString RBF120_16BIT_PATH = "../verilog/output_files/usb_test120_16bit.rbf";
    const QString RBF120_32BIT_PATH = "../verilog/output_files/usb_test120_32bit.rbf";

    const QString HEX_PATH = "../verilog/test_data_wire.hex";

// FTDI config
    #define FTDI_SESSION_UNDERRUN   (CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN | CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL)
    #define FTDI_FIFO_CLK           CONFIGURATION_FIFO_CLK_66
    #define FTDI_FIFO_MODE          CONFIGURATION_FIFO_MODE_245
    #define FTDI_CHANNEL_CONFIG     CONFIGURATION_CHANNEL_CONFIG_1

    static const char DEVICE_NAME120[]			= "v1 Z64_120";
    static const char DEVICE_NAME080[]			= "v1 Z64_80";

    static const char *g_chSerialNumber             = "000000000001";
    static const char VENDOR_NAME[]					= "JS Zaslon";

    static const quint32 WORDS_4SEND_BYTE = 16; // сколько нужно послать слов, чтобы передать один байт

// number of step progress bar in appropriate method:
    const UINT USB_RECONF = 6;
    const UINT USB_PROG = 4;

    const UINT FPGA_TEST = 8;

// for show part of recieved/transmit data array in log if test was success (for debug only):
    const UINT BUF_ROW_SHOW = 8;
    const UINT BUF_COL_SHOW = 4;

    const UINT MIN_BUF_SIZE = 2 * BUF_ROW_SHOW * BUF_COL_SHOW;

enum eFPGAPOWER {
    Pwr_OFF,
    Pwr_ON
};

#endif
