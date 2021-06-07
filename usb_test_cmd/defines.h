#ifndef DEFINES_H
#define DEFINES_H

#include <string>

using namespace std;

using ushort    = unsigned short;
using uchar     = unsigned char;
using ulong     = unsigned long;

enum eMainInfo{
    main_ok,
    main_err_reconf,
    main_err_prog,
    main_test_failed,
};

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
    //prog_err_reconf,
    prog_err_wr,
    prog_err_rd
};

enum eTestInfo{
    test_ok,
    test_err_open_file,
    test_err_arr_size,
    test_err_wr,
    test_err_rd,
    test_failed
};

enum eFPGAPOWER {
    Pwr_OFF,
    Pwr_ON
};

// main function arguments:
    static const char *ARG_HELP = "-h";

    static const char *ARG_FTDI601 = "-FT601"; // 32 bit data width
    static const char *ARG_FTDI600 = "-FT600"; // 16 bit

    static const char *ARG_FPGA120 = "-CL120"; // 120k LEs
    static const char *ARG_FPGA080 = "-CL080"; // 80k LEs

// default paths:
    const string RBF080_16BIT_PATH = "../verilog/output_files/usb_test080_16bit.rbf";
    const string RBF080_32BIT_PATH = "../verilog/output_files/usb_test080_32bit.rbf";
    const string RBF120_32BIT_PATH = "../verilog/output_files/usb_test120_32bit.rbf";

    const string HEX_PATH = "../verilog/test_data_wire.hex";

    const uint16_t HEX32_LENGTH = 8; // 1 byte - 2 symblos etc.
    const uint16_t HEX16_LENGTH = 4;

// FTDI config
    #define FTDI_SESSION_UNDERRUN   (CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN | CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL)
    #define FTDI_FIFO_CLK           CONFIGURATION_FIFO_CLK_66
    #define FTDI_FIFO_MODE          CONFIGURATION_FIFO_MODE_245
    #define FTDI_CHANNEL_CONFIG     CONFIGURATION_CHANNEL_CONFIG_1

    static const char DEVICE_NAME120[]			= "v1 Z64_120";
    static const char DEVICE_NAME080[]			= "v1 Z64_80";

    static const char *g_chSerialNumber             = "000000000001";
    static const char VENDOR_NAME[]					= "JS Zaslon";

    static const ulong WORDS_4SEND_BYTE = 16; // сколько нужно послать слов, чтобы передать один байт

// number of step progress bar in appropriate method:
    const uint16_t USB_RECONF = 6;
    const uint16_t USB_PROG = 4;
    const uint16_t FPGA_TEST = 7;

const uint16_t MIN_BUF_SIZE = 32;
const uint16_t BUF_SIZE = 256;
const uint16_t RPT_TEST = 5;

#endif
