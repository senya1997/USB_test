`timescale 1ns/100ps

`define FW_VER		32'd01_000		// xx.xxx
`define FW_DATE	32'd05_14_21	// mounth.day.year

// for tb:
	`define CLK_USB	100	// MHz (from FTDI600/601)
	`define CLK_FPGA	50		// MHz (async to 'CLK_USB')

	`define TACT_USB			$ceil(1000/`CLK_USB) // ns 
	`define HALF_TACT_USB	(`TACT_USB/2)

	`define TACT_FPGA			$ceil(1000/`CLK_FPGA) // ns 
	`define HALF_TACT_FPGA	(`TACT_FPGA/2)
