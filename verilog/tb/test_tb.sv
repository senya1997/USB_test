`include "../defines.v"

module test_tb;

bit clk;
bit reset;

logic [13 : 0] adc_data;
logic adc_rdy;

wire ADC_CONV_ST;

wire [15 : 0] FTDI_DATA;
wire [1 : 0] FTDI_BE;

wire TXE_N;

wire OE_N;
wire RD_N;
wire WR_N;

wire [1 : 0] GPIO;

initial begin
	$timeformat(-6, 3, " us", 6);
	clk = 1;
	forever	#(`HALF_TACT) clk = ~clk;
end

initial begin
 	reset = 1'b1; #(2*`TACT);
	reset = 1'b0; #(`TACT);
	reset = 1'b1;
end

initial begin
	adc_rdy = 1'b0;
	adc_data = 14'hzzz;
	#(10*`TACT);
	
	forever	begin
		wait(ADC_CONV_ST); #(`TACT);
		
		adc_data = $unsigned($random)%(14'h3FFF);

		adc_rdy = 1'b1; #(`TACT);
		adc_rdy = 1'b0; #(`TACT);
		
		adc_data = 14'hzzz;
	end
end

initial begin
	forever begin
		if(~WR_N & (DUT.RAM_BUF.altsyncram_component.m_default.altsyncram_inst.mem_data[FTDI_IMIT.data_cnt-1] != FTDI_IMIT.data))
			$display("\t*** error data in buf RAM and FTDI are different: %d, %d, %t", 
					FTDI_IMIT.data, DUT.RAM_BUF.altsyncram_component.m_default.altsyncram_inst.mem_data[FTDI_IMIT.data_cnt], $time);
		
		#(`TACT);
	end
end

test_ftdi_imit FTDI_IMIT(
	.iCLK(clk), // 100 or 66 MHz

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
	.iRESET(reset),
	
// ADC interface:
	.iADC_RDY(adc_rdy),
	.iADC_DATA(adc_data),
	
	.oADC_CONV_ST(ADC_CONV_ST),
	
// FTDI600 interface:
	.ioDATA(FTDI_DATA),
	.ioBE(FTDI_BE),

	.iTXE_N(TXE_N),
	
	.oOE_N(OE_N),
	.oRD_N(RD_N),
	.oWR_N(WR_N),
	
	.oGPIO(GPIO)
);

endmodule