`timescale 1ns/100ps

`define BUF_SIZE	10'd256 // max 1024 bytes from FTDI datasheet (32 bit - 256 word)

// for tb:

	//`define CLK_USB	66	// MHz from FTDI600/601
	//`define CLK_FPGA	50	// MHz async to 'CLK_USB'

	//`define TACT_USB			$ceil(1000/`CLK_USB) // ns 
	//`define HALF_TACT_USB	(`TACT_USB/2)

	//`define TACT_FPGA			$ceil(1000/`CLK_FPGA) // ns 
	//`define HALF_TACT_FPGA	(`TACT_FPGA/2)
	
	`define TACT_USB			15.16 // ns 
	`define HALF_TACT_USB	7.58
	
	`define TACT_FPGA			20 // ns 
	`define HALF_TACT_FPGA	10
	
	`define CLK_BIAS 4 // ns, random bias clk for async USB and FPGA clocks
	
	`define FTDI_DELAY 5_000 // ns dealy FTDI handling packet

// auto updated defines:
// do not touch!

	// xx.xxx two first - version, other - build number, allow to change only two first number (version):
		`define FW_VER 1033
		`define FW_DATE 06_02_21
