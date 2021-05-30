`include "../defines.v"

module test_ftdi_imit(
	input iRESET,
	output oCLK, // 100 or 66 MHz

	inout [31 : 0]	ioDATA,
	inout [3 : 0]	ioBE,
	
	output oTXE_N, // transmit FIFO buffer empty (active low)
	output oRXF_N,
	
	input iOE_N,
	input iRD_N,
	input iWR_N,
	
	input [1 : 0] iGPIO
);

logic clk_usb;

logic txe_n = 1'b1;
logic rxf_n = 1'b1;

logic [9 : 0] data_in_cnt = 10'd0;
logic [9 : 0] data_out_cnt = 10'd0;

logic [31 : 0] data_in;
logic [31 : 0] data_out = 32'd0;

logic [3 : 0] byte_en = 4'b0000;

//wire WRONG_BYTE_EN = ((ioBE === 4'bZZZZ) | (&(ioBE) === 1'bX));  // mb Z-state on last word permissible
wire WRONG_BYTE_EN = (&(ioBE) === 1'bX);

initial begin
	//int clk_bias = $urandom_range(1, 7); // avoid zero
	
	$timeformat(-6, 3, " us", 6);
	
	#(`CLK_BIAS) clk_usb = 1; // async

	forever	#(`HALF_TACT_USB) clk_usb = ~clk_usb;
end

// recieve data:
	always@(negedge iRESET)begin
		#(`FTDI_DELAY);
		wait(clk_usb);
		wait(~clk_usb);
		
		txe_n = 1'b0;
	end

/*****************************************************************************************************/

	always@(posedge clk_usb)begin
		if(~iWR_N & ~txe_n & (|ioBE))
			begin
				data_in_cnt = data_in_cnt + 1'b1;
				data_in = ioDATA;
			end
	end

	always@(data_in_cnt or posedge iWR_N)begin
		if((data_in_cnt == `BUF_SIZE) | iWR_N)
			begin
				$display("\tRecieve packet from master %d words: %t\n", data_in_cnt, $time);
				
				#(`TACT_USB)
				data_in_cnt = 10'd0;
				txe_n = 1'b1;
				
				#(`FTDI_DELAY);
				wait(clk_usb);
				wait(~clk_usb);
					
				txe_n = 1'b0;
			end
	end

/*****************************************************************************************************/

// transmit data:
	always@(negedge txe_n)begin // imit FTDI IN BUF from PC ready to transmit in FPGA
		#(`FTDI_DELAY);
		wait(clk_usb);
		wait(~clk_usb);
		
		rxf_n = 1'b0;
	end

	always@(negedge clk_usb)begin
		if(~iOE_N & ~rxf_n) 
			begin
				byte_en = 4'b1111;
				data_out = $unsigned($random)%(32'hFFFF_FFFF);
				
				if(~iOE_N & iRD_N) data_out_cnt = 10'd0; // init
				else data_out_cnt = data_out_cnt + 1'b1;
			end
	end

	always@(data_out_cnt)begin
		if(data_out_cnt == `BUF_SIZE)
			begin
				#(`TACT_USB);
				$display("\tTransmit packet to master %d words: %t\n", data_out_cnt, $time);
				wait(clk_usb);
				wait(~clk_usb);
				
				data_out_cnt = 10'd0;
				data_out = 32'd0;
				rxf_n = 1'b1;
			end
	end

// wrong inputs:
	always@(iGPIO)begin
		if(iGPIO == 2'b00) $display("\n\t245 Synchronous FIFO mode: %t\n", $time);
		else $display("***\tError FIFO mode: %t\n", $time);
	end
	
	always@(iWR_N, iOE_N, iRD_N)
		if(~iWR_N & (~iOE_N | ~iRD_N)) $display("***\tError WR_N can not equal zero with OE_N and/or RD_N on read/write cycle: %t\n", $time);

	always@(ioDATA) if(&(ioDATA) === 1'bX) $display("***\tError DATA bus is wrong: %t\n", $time);
	always@(ioBE) if(WRONG_BYTE_EN) $display("***\tError BYTE EN is wrong: %t\n", $time);

// outputs:
	assign oCLK = clk_usb;

	assign oTXE_N = txe_n;
	assign oRXF_N = rxf_n;

	assign ioDATA	= iWR_N ? data_out : 32'hZZZZ;
	assign ioBE		= iWR_N ? byte_en : 4'hZ;

endmodule