`include "../defines.v"

module test_tb;

bit clk;

wire CLK_USB;

wire [31 : 0] FTDI_DATA;
wire [3 : 0] FTDI_BE;

wire TXE_N;
wire RXF_N;

wire OE_N;
wire RD_N;
wire WR_N;

int pll_per = 0;

initial begin
	$timeformat(-6, 3, " us", 6);
	clk = 1;
	forever	#(`HALF_TACT_FPGA) clk = ~clk;
end

always@(negedge DUT.reset)begin // check PLL
	if(DUT.CLK_100 === 1'bx) $display("***\tError PLL do not work: %t\n", $time);
	else
		begin
			wait(DUT.CLK_100);
			
			while(DUT.CLK_100)
				begin
					if(pll_per >= `TACT_FPGA) break;
					#1 pll_per = pll_per + 1;
				end
			
			if(pll_per >= `TACT_FPGA) $display("***\tError PLL do not work: %t\n", $time);
			else $display("\tPLL is working: %t\n", $time);
		end
end

test_ftdi_imit FTDI_IMIT(
	.iRESET(DUT.reset),
	.oCLK(CLK_USB), // 100 or 66 MHz

	.ioDATA(FTDI_DATA), // bufer = 1 kB
	.ioBE(FTDI_BE),
	
	.oTXE_N(TXE_N),
	.oRXF_N(RXF_N),
	
	.iOE_N(OE_N),
	.iRD_N(RD_N),
	.iWR_N(WR_N),
	
	.iGPIO(2'b00)
);

test_top DUT(
	.iCLK(clk),
	
	.oLED(),
	.oTP(),
	
// FTDI600/601 interface (all signals active low):
	.iUSB_CLK(CLK_USB),
	
	.ioDATA(FTDI_DATA),
	.ioBE(FTDI_BE),

	.iTXE_N(TXE_N),
	.iRXF_N(RXF_N),
	
	.oOE_N(OE_N),
	.oRD_N(RD_N),
	.oWR_N(WR_N)
);

endmodule