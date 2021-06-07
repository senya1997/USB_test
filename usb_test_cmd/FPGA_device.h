#ifndef FPGA_DEVICE_H
#define FPGA_DEVICE_H

#include <fstream>
#include <chrono>
#include <thread>

#include "defines.h"
#include "USB_device.h"

class FPGA_device
{
    public:
           FPGA_device();
           ~FPGA_device();

           eInitInfo Initialize(bool force_reconf, bool ui_cyclone_LEs); // connect or reconfig FTDI

           eProgInfo Program(bool ui_data_width, ulong *ver, ulong *date); // return version and date of firmware readed from FPGA

           eTestInfo StartTest(string HexFileName, bool ui_data_width, ulong *num_err, ulong data_exp[], ulong data_err[]); // output if test failed: num of error words, expect data, wrong data

    private:
           USB_device *usb_device = nullptr;
};

#endif
