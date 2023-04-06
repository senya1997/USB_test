## Starting ##

Program using for test connect and check each bit on bus between FPGA Altera Cyclone V CL080 or CL120 and FTDI60x. FPGA program through FTDI using ```.rbf``` file.

### GUI version ###

1. At the startup, there is a connection to FTDI, reading the descriptor.

2. For the test, you first need to program the FPGA through the ```Program FPGA``` button.

3. If the FTDI has the correct descriptor, the firmware file is selected automatically based on the recorded descriptor
   (CL120 or CL080), in this case the choice is ```Cyclone LEs:``` radio button is ignored and will be set according to the descriptor.

4. If FTDI has an incorrect descriptor (when running in the ```FTDI description``` field will be ```Wrong descriptor```), the firmware FPGA and FTDI files is chooses from ```Cyclone LEs:``` radio button.

5. If an incorrect descriptor was written by mistake (CL080 was written for CL120 and vice versa), then you can write in FTDI
   the desired descriptor is forced through ```Force prog FTDI``` button and select the desired one through ```Cyclone LEs:``` radio button.

6. The selection of ```FTDI data width:``` (FT600/FT601) must always be done manually.

7. After successful FPGA programming, the FPGA firmware version and date are counted in the ```Firmware ver/date:``` fields.

8. Then you need to run the ```Start test``` button.

9. Basic information about errors and possible causes is displayed in the main window, and a chain is displayed in the ```show log```:
   function calls and an array of test data recorded in the FPGA, in case of a test error, broken data is output in the format
   "what was sent - what came"' in HEX code.

### Console version ###

* Keys: order and case are not important, one or two keys can be set, FT601 and CL080 are used by default when starting without parameters.
* The CL120 and FT600 option is not used - one of the parameters will be replaced.
* All information about the work and test data is written to the log file, in case of a test error, broken data is written in the format "what was sent - what came"' in HEX code.

***
	Choose FPGA: -CL120 or -CL080
	Choose FTDI: -FT600 or -FT601
***

### Comments ###

##### Verilog compilation: #####

* In tcl ```usb_test``` - the ```.rbf``` of all revisions will be updated.
* When compile individual revisions, tcl ```pre_usb_test``` will be executed before starting.

##### Remarks: #####

* Build number in the define file is one for all, i.e. when compiling all revisions, build number is incremented for each separately and all output files will be different.
* If it was not possible to flash the FPGA the first time - the test on the first iteration may give an error, because there is garbage in the FTDI buffer, so should run the test several times.