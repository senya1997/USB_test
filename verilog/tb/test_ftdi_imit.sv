`timescale 1ns/1ns

`define FTDI_DELAY 100_000 // ns (dealy FTDI handling packet)

module test_ftdi_imit(
	input iCLK, // 100 or 66 MHz

	inout [31 : 0]	ioDATA, // bufer = 1 kB
	inout [3 : 0]	ioBE,
	
	output oTXE_N, // transmit FIFO buffer empty (active low)
	
	input iOE_N,
	input iRD_N,
	input iWR_N,
	
	input [1 : 0] iGPIO
);

initial $timeformat(-6, 3, " us", 6);

logic txe_n = 1'b0;
logic [9 : 0] data_cnt = 10'd0;
logic [31 : 0] data;

always@(iGPIO, iOE_N, iRD_N)begin
	if(iGPIO == 2'b00) $display("\n\t245 Synchronous FIFO mode: %t\n", $time);
	else
		begin
			$display("\n*** error FIFO mode: %t\n", $time);
		end
		
	if(~iOE_N | ~iRD_N)
		begin
			$display("\n*** error OE_N and RD_N must be '1' on write cycle: %t\n", $time);
		end
end

always@(posedge iCLK)begin
	if(iWR_N & (data_cnt != 10'd0))
		begin
			$display("\n*** error breakdown of packet (1 packet - 1 kB): %t\n", $time);
		end
	else if(~iWR_N)
		begin
			data_cnt = data_cnt + 1'b1;
			data = ioDATA;
		end
end

always@(data_cnt)begin
	if(data_cnt == 10'd1023) 
		begin
			txe_n = 1'b1;
			#(`FTDI_DELAY);
			txe_n = 1'b0;

			$display("\ttransmit 1 packet: %t (press 'run')", $time);		
		end
end

assign oTXE_N = txe_n;

endmodule