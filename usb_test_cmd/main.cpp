#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>

#include <iomanip>

#include "defines.h"
#include "FPGA_device.h"

int main(int argc, char* argv[])
{
    FPGA_device *fpga = new FPGA_device();

    eInitInfo fpga_init;
    eProgInfo fpga_prog;
    eTestInfo fpga_test;

    ulong ver, date;
    ulong num_err;

    ulong data_err[BUF_SIZE];
    ulong data_exp[BUF_SIZE];

// default:
    string data_width_str;
    string cyclone_LEs_str;

    bool data_width = false; // false - 32 bit (FT601)
    bool cyclone_LEs = false; // false - 80k

    bool data_width_arg = false; // flag - args readed and changed param
    bool cyclone_LEs_arg = false;

ofstream log_file;

char buf[16]; // length of string: "%m.%d.%y_%H.%M"
time_t now;

string temp_str;

now = chrono::system_clock::to_time_t(chrono::system_clock::now());
strftime(buf, sizeof(buf), "%m.%d.%y_%H.%M", localtime(&now));

temp_str = "log_";
temp_str += buf;
temp_str += ".txt";

log_file.open(temp_str, ios::out);

/********************************************************************************************************************/
/*                                              Read input args:                                                    */
/********************************************************************************************************************/

    temp_str = "\n\tFTDI60x - FPGA CONNECT TEST\n\n";

    cout << temp_str;
    log_file << temp_str;

    if(argc > 1)
    {
        if(strcasecmp(argv[1], ARG_FTDI601) == 0)
        {
            data_width_arg = true;
            data_width  = false;
        }
        else if(strcasecmp(argv[1], ARG_FTDI600) == 0)
        {
            data_width_arg = true;
            data_width  = true;
        }
        else if(strcasecmp(argv[1], ARG_FPGA080) == 0)
        {
            cyclone_LEs_arg = true;
            cyclone_LEs = false;
        }
        else if(strcasecmp(argv[1], ARG_FPGA120) == 0)
        {
            cyclone_LEs_arg = true;
            cyclone_LEs = true;
        }

        if(argc > 2)
        {
            if(data_width_arg & !data_width) // if 16 bit LEs don't change for avoid 'FT600' + 'CL120'
            {
                if(strcasecmp(argv[2], ARG_FPGA080) == 0) cyclone_LEs = false;
                else if(strcasecmp(argv[2], ARG_FPGA120) == 0) cyclone_LEs = true;
            }
            else if(cyclone_LEs_arg & !cyclone_LEs) // if LEs 120k data width don't change for avoid 'FT600' + 'CL120'
            {
                if(strcasecmp(argv[2], ARG_FTDI601) == 0) data_width  = false;
                else if(strcasecmp(argv[2], ARG_FTDI600) == 0) data_width = true;
            }
        }
    }

    if(data_width)  data_width_str = ARG_FTDI600;
    else            data_width_str = ARG_FTDI601;

    if(cyclone_LEs) cyclone_LEs_str = ARG_FPGA120;
    else            cyclone_LEs_str = ARG_FPGA080;

    temp_str = "Used parameters: " + data_width_str + ", " + cyclone_LEs_str + "\n";

    cout << temp_str;
    log_file << temp_str;

/********************************************************************************************************************/
/*                                              Connect to FTDI:                                                    */
/********************************************************************************************************************/

    temp_str = "Connect FTDI:\n";

    cout << temp_str;
    log_file << temp_str;

    for(uint16_t i = 0; i < RPT_TEST; ++i)
    {
        fpga_init = fpga->Initialize(true, cyclone_LEs);

        if(fpga_init == init_ok)
        {
            temp_str = "\tFTDI connected!\n";

            cout << temp_str;
            log_file << temp_str;

            break;
        }
        else
        {
            temp_str = "\tFailed! Try to reconfig...\n";

            cout << temp_str;
            log_file << temp_str;

            this_thread::sleep_for(1s);
        }
    }

    if(fpga_init != init_ok)
    {
        temp_str = "\n\tFailed FTDI reconfig!\n";

        cout << temp_str;
        log_file << temp_str;

        log_file.close();
        delete fpga;

        return main_err_reconf;
    }

/********************************************************************************************************************/
/*                                               Program FPGA:                                                      */
/********************************************************************************************************************/

    temp_str = "Program FPGA:\n";

    cout << temp_str;
    log_file << temp_str;

    for(uint16_t i = 0; i < RPT_TEST; ++i)
    {
        fpga_prog = fpga->Program(data_width, &ver, &date);

        if(fpga_prog == prog_ok) // required 'setfill' do through string
        {
            cout << "\tFirmware ver: " << setfill('0') << setw(2) << ver/1000 << "." <<
                                          setfill('0') << setw(3) << ver%1000 << "\n";
            cout << "\tFirmware date: " << setfill('0') << setw(2) << date/10'000 << "." <<
                                           setfill('0') << setw(2) << (date%10'000)/100 << "." <<
                                           setfill('0') << setw(2) << date%100 << "\n";

            log_file << "\tFirmware ver: " << setfill('0') << setw(2) << ver/1000 << "." <<
                                              setfill('0') << setw(3) << ver%1000 << "\n";
            log_file << "\tFirmware date: " << setfill('0') << setw(2) << date/10'000 << "." <<
                                               setfill('0') << setw(2) << (date%10'000)/100 << "." <<
                                               setfill('0') << setw(2) << date%100 << "\n";

            break;
        }
        else
        {
            temp_str = "\tFailed! Try to program again...\n";

            cout << temp_str;
            log_file << temp_str;

            this_thread::sleep_for(1s);
        }
    }

    if(fpga_prog != prog_ok)
    {
        switch (fpga_prog)
        {
            case prog_err_open_file:    temp_str = "\n\tError open '.rbf'!\n\tRelative path must be like e.g.: " + RBF080_32BIT_PATH + "\n"; break;
            case prog_err_wr:           temp_str = "\n\tError write FTDI! Check power\n"; break;
            case prog_err_rd:           temp_str = "\n\tError read FTDI! Check data width\n"; break;

            default: temp_str = "\n\tError! Code:\n" + to_string(fpga_prog); break;
        }

        cout << temp_str;
        log_file << temp_str;

        log_file.close();
        delete fpga;

        return main_err_prog;
    }

/********************************************************************************************************************/
/*                                               Start test:                                                        */
/********************************************************************************************************************/

    temp_str = "Start test USB - FPGA:\n";

    cout << temp_str;
    log_file << temp_str;

    for(uint16_t i = 0; i < RPT_TEST; ++i)
    {
        fpga_test = fpga->StartTest(HEX_PATH, data_width, &num_err, &data_exp[0], &data_err[0]); // first element array - pointer on this array

        if(fpga_test == test_ok)
        {
            temp_str = "\tSuccess! Test done\n";

            cout << temp_str;
            log_file << temp_str;

            log_file.close();
            delete fpga;

            return main_ok;
        }
        else
        {
            temp_str = "\tFailed! Run test again...\n";

            cout << temp_str;
            log_file << temp_str;

            this_thread::sleep_for(1s);
        }
    }

    if(fpga_test != test_ok)
    {
        switch (fpga_test)
        {
            case test_err_open_file:    temp_str = "\n\tError open '.hex'!\n\tRelative path must be: " + HEX_PATH + "\n"; break;
            case test_err_arr_size:     temp_str = "\n\tError hex buf size!\n"; break;
            case test_err_wr:           temp_str = "\n\tError write FTDI!\n"; break;
            case test_err_rd:           temp_str = "\n\tError read FTDI!\n\tWas FPGA program?\n"; break;
            case test_failed:           temp_str = "\n\tTest failed!\n"; break;

            default: temp_str = "\n\tError! Code:\n" + to_string(fpga_test); break;
        }

        cout << temp_str;
        log_file << temp_str;

        log_file << "\nnum: transmit -> recieve (HEX)\n\n";

        log_file.unsetf(ios::dec);
        log_file.setf(ios::hex | ios::uppercase);

        for(uint16_t i = 0; i < num_err; ++i)
            log_file << i << ": " << setfill('0') << setw(8) << data_exp[i] << " -> " <<
                                     setfill('0') << setw(8) << data_err[i] << "\n";

        log_file.close();
        delete fpga;

        return main_test_failed;
    }
}
