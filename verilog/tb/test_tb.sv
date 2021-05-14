`include "../defines.v"

module test_tb;

bit clk, clk_usb;

wire [31 : 0] FTDI_DATA;
wire [3 : 0] FTDI_BE;

wire TXE_N;

wire OE_N;
wire RD_N;
wire WR_N;

wire [1 : 0] GPIO;

initial begin
	$timeformat(-6, 3, " us", 6);
	clk = 1;
	forever	#(`HALF_TACT_FPGA) clk = ~clk;
end

initial begin
	$timeformat(-6, 3, " us", 6);
	#3 clk_usb = 1; // async
	forever	#(`HALF_TACT_USB) clk_usb = ~clk_usb;
end

test_ftdi_imit FTDI_IMIT(
	.iCLK(clk_usb), // 100 or 66 MHz

	.ioDATA(FTDI_DATA), // bufer = 1 kB
	.ioBE(FTDI_BE),
	
	.oTXE_N(TXE_N),
	
	.iOE_N(OE_N),
	.iRD_N(RD_N),
	.iWR_N(WR_N),
	
	.iGPIO(GPIO)
);

test_top DUT(
	.iCLK(clk),
	
// FTDI600/601 interface (all signals active low):
	.iUSB_CLK(clk_usb),
	
	.ioDATA(FTDI_DATA),
	.ioBE(FTDI_BE),

	.iTXE_N(TXE_N),
	.iRXF_N(),
	
	.oOE_N(OE_N),
	.oRD_N(RD_N),
	.oWR_N(WR_N)
);

endmodule