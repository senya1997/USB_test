`timescale 1ns/100ps

`define CLK_USB	100 // MHz (from FTDI600/601)
`define CLK_FPGA	100 // MHz (async to 'CLK_USB')



`define TACT_USB			$ceil(1000/`CLK_USB) // ns 
`define HALF_TACT_USB	(`TACT_USB/2)

`define TACT_FPGA			$ceil(1000/`CLK_FPGA) // ns 
`define HALF_TACT_FPGA	(`TACT_FPGA/2)
