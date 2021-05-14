`include "defines.v"

module test_top (
	input iCLK,
	
	output reg [2 : 0] oLED,
	output reg [4 : 0] oTP,
	
// FTDI600/601 interface (all signals active low):
	input iUSB_CLK,
	
	inout [31 : 0] ioDATA,
	inout [3 : 0] ioBE, // byte enable

	input iTXE_N, // transmit FIFO buffer empty
	input iRXF_N,
	
	output oOE_N,
	output oRD_N,
	output oWR_N
);

reg [24 : 0] cnt_led = 0;

initial oLED = 3'b111;
initial oTP = 5'b00000;

reg reset;
reg [9 : 0] cnt_reset = 10'd0;

reg [31 : 0] ver; // x.xx in decimal
reg [31 : 0] date; // mm.dd.yy in decimal

reg [2 : 0] shift_txe_n;
reg [2 : 0] shift_rxf_n;

reg oe_n;
reg rd_n;
reg wr_n;

reg [3 : 0] state;

//reg [3 : 0] byte_en;

reg flag_buf_out; // 1 - mean that data was loaded in OUT FIFO, 0 - data is loading
reg flag_out;

// states:
	localparam S_IDLE = 4'd0;
	
	localparam S_WR_1 = 4'd1;
	localparam S_WR_2 = 4'd2;

	localparam S_RD_1 = 4'd3;
	localparam S_RD_2 = 4'd4;
	localparam S_RD_3 = 4'd5;
	localparam S_RD_4 = 4'd6;

	localparam S_END = 4'd7;

wire CLK_100;

// out FIFO:
	reg [31 : 0]	buf_out_data;
	
	reg [9 : 0]		buf_out_num_data;
	reg [9 : 0]		buf_out_cnt_data; // buf size 1024

	reg buf_out_rd; // negedge 'iUSB_CLK'
	reg buf_out_wr; // posedge 'iCLK'

	wire [31 : 0] DATA_OUT;
	
	wire BUF_OUT_EMPTY, BUF_OUT_FULL;

	wire BUF_OUT_LOADED = (buf_out_cnt_data == buf_out_num_data);
	wire BUF_OUT_STORE = 1'b0;
	
// in FIFO:
	wire [31 : 0] DATA_IN;
	
	wire BUF_IN_EMPTY, BUF_IN_FULL;

/***************************************************************************************************/
/*														services regs:															*/
/***************************************************************************************************/

always@(posedge iCLK)begin
	cnt_led <= cnt_led + 1'b1;

	oLED[0] <= 1'b0;
	oLED[1] <= 1'b1;
	
	if(cnt_led == 25'd0) oLED[2] <= ~oLED[2];
	
	oTP[0] <= ~oTP[0];
	
	oTP[1] <= 1'b0;
	oTP[2] <= 1'b1;
end

always@(posedge iCLK)begin
	if(cnt_reset == 10'd128) reset <= 1'b0;
	else 
		begin
			reset	<=	1'b1;
			cnt_reset <= cnt_reset + 1'b1;		
		end       
end
	
always@(posedge CLK_100 or posedge reset)begin
	if(reset) 
		begin
			ver <= 32'd0;
			date <= 32'd0;
		end
	else
		begin
			ver	<= `FW_VER;
			date	<= `FW_DATE;
		end
end
	
/***************************************************************************************************/
/*														posedge CLK:															*/
/***************************************************************************************************/

always@(posedge CLK_100 or posedge reset)begin
	if(reset) flag_buf_out <= 1'b0;
	else if(BUF_OUT_LOADED) flag_buf_out <= 1'b1;
	else flag_buf_out <= 1'b0;
end

always@(posedge CLK_100 or posedge reset)begin
	if(reset) buf_out_wr <= 1'b0;
	else if(!BUF_OUT_LOADED) buf_out_wr <= 1'b1;
	else buf_out_wr <= 1'b0;
end

always@(posedge CLK_100 or posedge reset)begin
	if(reset) buf_out_data <= 32'd0;
	else
		begin
			// DEBUG:
				if(buf_out_cnt_data[0] == 1'b0) buf_out_data <= ver;
				else if(buf_out_cnt_data[0] == 1'b1) buf_out_data <= date;
		end
end

always@(posedge CLK_100 or posedge reset)begin
	if(reset) buf_out_cnt_data <= 10'd0;
	else if(!BUF_OUT_LOADED) buf_out_cnt_data <= buf_out_cnt_data + 1'b1;
	else if(BUF_OUT_STORE) buf_out_cnt_data <= 10'd0;
end

always@(posedge CLK_100 or posedge reset)begin
	if(reset) buf_out_num_data <= 10'd0;
	else buf_out_num_data <= 10'd2; // DEBUG
end

/***************************************************************************************************/
/*												posedge USB clock:															*/
/***************************************************************************************************/

always@(posedge iUSB_CLK or posedge reset)begin
	if(reset)
		begin
			shift_txe_n <= 3'b111;
			shift_rxf_n <= 3'b111;
		end
	else
		begin
			shift_txe_n <= {shift_txe_n[1 : 0], iTXE_N};
			shift_rxf_n <= {shift_rxf_n[1 : 0], iRXF_N};
		end
end

/***************************************************************************************************/
/*												negedge USB clock:															*/
/***************************************************************************************************/

always@(negedge iUSB_CLK or posedge reset)begin    
	if(reset) buf_out_rd <= 1'b0;
	else if(!shift_txe_n[2] & flag_buf_out) buf_out_rd <= 1'b1;
	else buf_out_rd <= 1'b0;
end

always@(negedge iUSB_CLK or posedge reset)begin    
	if(reset) flag_out <= 1'b0;
	else if(!shift_txe_n[1] & flag_buf_out) flag_out <= 1'b1;
	else flag_out <= 1'b0;
end

always@(negedge iUSB_CLK or posedge reset)begin    
	if(reset) 
		begin
			//byte_en  <= 4'b1111;
			
			oe_n <= 1'b1;
			rd_n <= 1'b1;
			wr_n <= 1'b1;
			
			state <= S_IDLE;
		end 
	else
		case (state)
			S_IDLE:   
				begin
					if(!shift_txe_n[2] & !BUF_OUT_EMPTY & flag_buf_out) state <= S_WR_1;
					//else if(!shift_rxf_n[2] & !com_fifo_full) state <= RD1;
					else state <= S_IDLE;
				end
			
		// write in FTDI:
			S_WR_1:
				begin
					if(!shift_txe_n[2] & !BUF_OUT_EMPTY)
						begin
							wr_n <= 1'b0;
							state <= S_WR_1;        
						end
					else state <= S_WR_2;
				end                                  
			S_WR_2: 
				begin
					wr_n <= 1'b1;
					state <= S_IDLE;
				end
		/*	
		// read from FTDI:
			RD1:    begin
						 state <= RD2;
					end
			RD2:    begin
						 state <= RD3;
					end
			RD3:    begin
						 usb_oe <= 1'b1;
						 state <= RD4;
					end                
			RD4:    begin
						 if(usb_rxf && (!com_fifo_full)) begin
							  usb_rd <=1'b1;
							  state <= RD4;
						 end
						 else begin 
							  usb_oe <= 1'b0;
							  usb_rd <= 1'b0;
							  state  <= RD5;
						 end    
					end
			*/
			S_END: state <= S_IDLE;
			
			default: state <= S_IDLE;
		endcase
end

test_fifo FIFO_BUF_IN(
	.aclr(reset),
	
	.rdclk(CLK_100),
	.rdreq(),
	.q(DATA_IN),
	
	.wrclk(iUSB_CLK),
	.wrreq(),
	.data(ioDATA),
	
	.rdempty(BUF_IN_EMPTY),
	.wrfull(BUF_IN_FULL)
);

test_fifo FIFO_BUF_OUT(
	.aclr(reset),
	
	.rdclk(iUSB_CLK),
	.rdreq(buf_out_rd & !BUF_OUT_EMPTY),
	.q(DATA_OUT),
	
	.wrclk(CLK_100),
	.wrreq(buf_out_wr & !BUF_OUT_FULL),
	.data(buf_out_data),
	
	.rdempty(BUF_OUT_EMPTY),
	.wrfull(BUF_OUT_FULL)
);

test_pll PLL_100(
	.inclk0(iCLK),
	.areset(),
	
	.c0(CLK_100)
);
	
// outputs:
	assign ioDATA	= flag_out ? DATA_OUT : 32'hz;
	assign ioBE		= flag_out ? 4'b1111 : 4'hz;

	assign oOE_N = oe_n;
	assign oRD_N = rd_n;
	assign oWR_N = wr_n;

endmodule 