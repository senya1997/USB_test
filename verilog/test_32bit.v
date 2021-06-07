`include "defines.v"

module test_32bit #(parameter D_BIT = 32, BE_BIT = 4)(
	input iCLK,
	
// for debug:
	input [1 : 0] iTP,
	output [2 : 0] oTP,
	
	output [2 : 0] oLED,
	
// FTDI600/601 interface (all signals active low):
	input iUSB_CLK,
	
	inout [D_BIT - 1 : 0] ioDATA,
	inout [BE_BIT - 1 : 0] ioBE, // byte enable

	input iTXE_N, // transmit FIFO buffer empty
	input iRXF_N,
	
	output oOE_N,
	output oRD_N,
	output oWR_N
);

wire [2 : 0] LED;
wire [2 : 0] TP;

wire OE_N;
wire RD_N;
wire WR_N;

test_top #(.D_BIT(D_BIT), .BE_BIT(BE_BIT)) TEST_TOP(
	.iCLK(iCLK),
	
	.oLED(LED),
	
	.iTP(iTP),
	.oTP(TP),
	
	.iUSB_CLK(iUSB_CLK),
	
	.ioDATA(ioDATA),
	.ioBE(ioBE),

	.iTXE_N(iTXE_N),
	.iRXF_N(iRXF_N),
	
	.oOE_N(OE_N),
	.oRD_N(RD_N),
	.oWR_N(WR_N)
);

assign oTP = TP;
assign oLED = LED;

assign oOE_N = OE_N;
assign oRD_N = RD_N;
assign oWR_N = WR_N;

endmodule 