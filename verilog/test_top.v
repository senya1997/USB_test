`include "defines.v"

module test_top (
	input iCLK,
	
// for debug:
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

/***************************************************************************************************/
/*																	regs:															*/
/***************************************************************************************************/

// services regs:
	reg reset; // using in tb
	reg [1 : 0] reset_usb; // sync CDC
	
	reg [9 : 0] cnt_reset;
	reg [24 : 0] cnt_led;

// FTDI interface:
	reg [3 : 0] byte_en; // mb Z-state on last word permissible like '4'b0000': tri-state port go to Z-state and this reg not needed

	reg [2 : 0] shift_txe_n;
	reg [2 : 0] shift_rxf_n;

	reg oe_n;
	reg rd_n;
	reg wr_n;

// out FIFO:
	reg [31 : 0]	buf_out_data;
	
	reg [9 : 0]		buf_out_num_data;
	reg [9 : 0]		buf_out_cnt_data; // defined by buf size words (max 1024 bytes in 1 USB packet from FTDI datasheet)

	reg buf_out_rd; // negedge 'iUSB_CLK'
	reg buf_out_wr; // posedge 'iCLK'
	
// in FIFO:
	reg buf_in_rd; // posedge 'iCLK'
	reg buf_in_wr; // posedge 'iUSB_CLK'
	
// flags:
	reg flag_upd_fw_n;			// 1 - mean that firmware data was updated and was load in FIFO
	reg flag_upd_buf_out;		// 1 - mean that need load data in out buf from: firmware data, in buf, etc.
	
	reg flag_send_buf_out;		// 1 - mean that buf out was updated and need to send in FTDI
	
	reg flag_buf_in_loaded;		// 1 - mean that need read buf in (and copy elements in out buf e.g.)
	reg flag_buf_out_loaded;	// 1 - flag use for CDC delay: wait when 'flag_buf_in_loaded_clk100' will equal zero
	
reg flag_buf_in_loaded_d, flag_buf_in_loaded_l;
reg [1 : 0] flag_buf_in_loaded_clk100; // required set [2 : 0] for better CDC and dont use '0' bit of this signal

// others:
	reg [2 : 0] state; // on USB clock
	
	reg [2 : 0] buf_out_loaded_clk100;
	reg [1 : 0] buf_out_loaded; // from FPGA to USB clock
	reg buf_out_loaded_l;
	
localparam	S_IDLE = 3'd0,

				S_WR_DATA	= 3'd1,
				S_WR_END		= 3'd2,

				S_RD_OE		= 3'd3,
				S_RD_RD		= 3'd4,
				S_RD_DATA	= 3'd5,
				S_RD_END 	= 3'd6;

/***************************************************************************************************/
/*																wires:															*/
/***************************************************************************************************/
					
// out FIFO:
	wire [31 : 0] DATA_OUT;
	
	wire BUF_OUT_EMPTY, BUF_OUT_FULL;

	wire BUF_OUT_LOADED = (buf_out_cnt_data == buf_out_num_data);
	
// in FIFO:
	wire [31 : 0] DATA_IN;
	
	wire BUF_IN_EMPTY, BUF_IN_FULL;

// others:
	wire CLK_100; // using in tb
	wire RESET_USB = reset_usb[1];
	
	//wire STATE_WR_DATA	= (state == S_WR_DATA);
	wire STATE_WR_END		= (state == S_WR_END);
	
	wire STATE_RD_RD		= (state == S_RD_RD);
	//wire STATE_RD_DATA	= (state == S_RD_DATA);
	wire STATE_RD_END 	= (state == S_RD_END);
	
	wire START_WR = (!shift_txe_n[2] & !BUF_OUT_EMPTY & flag_send_buf_out);
	wire START_RD = (!shift_rxf_n[2] & !BUF_IN_FULL & ~flag_buf_in_loaded);
	
	wire END_RD = (shift_rxf_n[0] | BUF_IN_FULL | (~(&ioBE)));
	
/***************************************************************************************************/
/*														services regs:															*/
/***************************************************************************************************/

initial begin
	cnt_reset = 10'd0;
	cnt_led = 25'd0;
	
	oLED = 3'b111;
	oTP = 5'b00000;
end

always@(posedge iCLK)begin
	cnt_led <= cnt_led + 1'b1;

	if(cnt_led[24]) oLED[0] <= ~oLED[0];
	if(cnt_led[23]) oLED[1] <= ~oLED[1];
	if(cnt_led[22]) oLED[2] <= ~oLED[2];
end

always@(posedge iCLK)begin
	if(cnt_reset == 10'd128) reset <= 1'b0;
	else 
		begin
			reset	<=	1'b1;
			cnt_reset <= cnt_reset + 1'b1;		
		end       
end

/***************************************************************************************************/
/*														posedge CLK:															*/
/***************************************************************************************************/

// flags:
	always@(posedge CLK_100 or posedge reset)begin
		if(reset) flag_upd_fw_n <= 1'b0;
		else if(buf_out_cnt_data != 10'd0 & BUF_OUT_LOADED) flag_upd_fw_n <= 1'b1;
	end
	
	always@(posedge CLK_100 or posedge reset)begin
		if(reset) flag_upd_buf_out <= 1'b0;
		else if(flag_buf_out_loaded | BUF_OUT_LOADED) flag_upd_buf_out <= 1'b0; // reset if buf out was load
		else if(flag_buf_in_loaded_clk100[1] | ~flag_upd_fw_n) flag_upd_buf_out <= 1'b1;
	end

	always@(posedge CLK_100 or posedge reset)begin
		if(reset) flag_buf_out_loaded <= 1'b0;
		else if(BUF_OUT_LOADED) flag_buf_out_loaded <= 1'b1;
		else if(~flag_buf_in_loaded_clk100[1]) flag_buf_out_loaded <= 1'b0; // reset
	end
	
// data, num of data, cnt data:
	always@(posedge CLK_100 or posedge reset)begin
		if(reset) buf_out_data <= 32'd0;
		else if(~flag_upd_fw_n)
			begin
				if(buf_out_cnt_data == 10'd0) buf_out_data <= `FW_VER;
				else if(buf_out_cnt_data == 10'd1) buf_out_data <= `FW_DATE;
			end
		else if(flag_buf_in_loaded_clk100[1]) buf_out_data <= DATA_IN;
	end

	always@(posedge CLK_100 or posedge reset)begin
		if(reset) buf_out_wr <= 1'b0;
		else if(flag_upd_buf_out) buf_out_wr <= 1'b1;
		else buf_out_wr <= 1'b0;
	end
	
	always@(posedge CLK_100 or posedge reset)begin
		if(reset) buf_out_cnt_data <= 10'd0;
		else if(flag_upd_buf_out & !BUF_OUT_LOADED) buf_out_cnt_data <= buf_out_cnt_data + 1'b1;
		else buf_out_cnt_data <= 10'd0;
	end

	always@(posedge CLK_100 or posedge reset)begin
		if(reset) buf_out_num_data <= 10'd1;
		else if(~flag_upd_fw_n) buf_out_num_data <= 10'd1;
		else buf_out_num_data <= `BUF_SIZE;
	end

	
	
	
	
/***************************************************************************************************/
	
	always@(posedge CLK_100 or posedge reset)begin
		if(reset) buf_in_rd <= 1'b0;
		else if(flag_buf_in_loaded_clk100[0]) buf_in_rd <= 1'b1;
		else buf_in_rd <= 1'b0;
	end
	
/***************************************************************************************************/
	
	
	
	
	
/***************************************************************************************************/
/*												posedge USB clock:															*/
/***************************************************************************************************/

always@(posedge iUSB_CLK or posedge RESET_USB)begin
	if(RESET_USB)
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

// flags:
	always@(negedge iUSB_CLK or posedge RESET_USB)begin    
		if(RESET_USB) flag_send_buf_out <= 1'b0;
		else if(buf_out_loaded[1]) flag_send_buf_out <= 1'b1;
		else if(STATE_WR_END) flag_send_buf_out <= 1'b0;
	end
	
	always@(negedge iUSB_CLK or posedge RESET_USB)begin
		if(RESET_USB) flag_buf_in_loaded <= 1'b0;
		else if(buf_out_loaded[1]) flag_buf_in_loaded <= 1'b0;
		else if(STATE_RD_END) flag_buf_in_loaded <= 1'b1;
	end
	
always@(negedge iUSB_CLK or posedge RESET_USB)begin    
	if(RESET_USB) 
		begin
			byte_en  <= 4'b0000;
			
			oe_n <= 1'b1;
			rd_n <= 1'b1;
			wr_n <= 1'b1;
			
			state <= S_IDLE;
		end 
	else
		case (state)
			S_IDLE:   
				begin
					if(START_WR) state <= S_WR_DATA; // sequence of 'if->elseif->else' defined priority of data from FPGA or PC
					else if(START_RD) state <= S_RD_OE;
					else state <= S_IDLE;
				end
			
		// write in FTDI:
			S_WR_DATA:
				begin
					wr_n <= 1'b0;
					byte_en  <= 4'b1111; // required check in tb conformity between valid DATA and BYTE EN
					
					if(shift_txe_n[0] | BUF_OUT_EMPTY)
						begin
							state <= S_WR_END;
							byte_en  <= 4'b0000; // all bytes in word (data) not valid
						end
				end                                  
			S_WR_END: 
				begin
					wr_n <= 1'b1;
					state <= S_IDLE;
				end
			
		// read from FTDI:
			S_RD_OE:
				begin
					state <= S_RD_RD;
					oe_n <= 1'b0;
				end
			S_RD_RD:   
				begin
					rd_n <= 1'b0;
					state <= S_RD_DATA;
				end
			S_RD_DATA: if(END_RD) state <= S_RD_END; // check if byte en will show that end packet first (mb replace by equal)
			S_RD_END: 
				begin 
					oe_n <= 1'b1;
					rd_n <= 1'b1;
					
					state <= S_IDLE;
				end  
			
			default: state <= S_IDLE;
		endcase
end

always@(negedge iUSB_CLK or posedge RESET_USB)begin    
	if(RESET_USB) buf_out_rd <= 1'b0;
	else if(START_WR) buf_out_rd <= 1'b1;
	else buf_out_rd <= 1'b0;
end

always@(negedge iUSB_CLK or posedge RESET_USB)begin    
	if(RESET_USB) buf_in_wr <= 1'b0;
	else if(STATE_RD_RD) buf_in_wr <= 1'b1; // mb required START_RD like 'buf_out_rd'
	else if(END_RD) buf_in_wr <= 1'b0;
end

// CDC:
	always@(negedge iUSB_CLK) reset_usb <= {reset_usb[0], reset};
	
	always@(posedge CLK_100 or posedge reset)begin    
		if(reset) 
			begin
				buf_out_loaded_clk100 <= 3'd0;
				buf_out_loaded_l <= 1'd0;
				
				flag_buf_in_loaded_clk100 <= 2'b00;
			end
		else
			begin
				buf_out_loaded_clk100 <= {buf_out_loaded_clk100[1 : 0], BUF_OUT_LOADED};
				buf_out_loaded_l <= |buf_out_loaded_clk100;
				
				flag_buf_in_loaded_clk100 <= {flag_buf_in_loaded_clk100[0], flag_buf_in_loaded_l}; // latch by fast clk
			end
	end
	
	always@(negedge iUSB_CLK or posedge RESET_USB)begin    
		if(RESET_USB)
			begin
				buf_out_loaded <= 2'b00;
				
				flag_buf_in_loaded_d <= 1'b0;
				flag_buf_in_loaded_l <= 1'b0;
			end
		else
			begin
				buf_out_loaded <= {buf_out_loaded[0], buf_out_loaded_l}; // latch by slow clk
				
				flag_buf_in_loaded_d <= flag_buf_in_loaded;
				flag_buf_in_loaded_l <= (flag_buf_in_loaded | flag_buf_in_loaded_d);
			end
	end
	
// instances:

	test_fifo FIFO_BUF_IN(
		.aclr(reset),
		
	// posedge CLK 100 MHz
		.rdclk(CLK_100),
		.rdreq(buf_in_rd & !BUF_IN_EMPTY),
		.q(DATA_IN),
		
		.wrclk(iUSB_CLK), // latch data on posedge USB CLK
		.wrreq(buf_in_wr & !BUF_IN_FULL), // negedge USB CLK
		.data(ioDATA),
		
		.rdempty(BUF_IN_EMPTY),
		.wrfull(BUF_IN_FULL)
	);

	test_fifo FIFO_BUF_OUT(
		.aclr(reset),
		
	// negedge USB CLK
		.rdclk(~iUSB_CLK), // WARNING TIMEQUEST!!!
		.rdreq(buf_out_rd & !BUF_OUT_EMPTY),
		.q(DATA_OUT),
		
	// posedge CLK 100 MHz
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

	assign ioDATA	= wr_n ? 32'hZZZZ : DATA_OUT;
	assign ioBE		= wr_n ? 4'hZ : byte_en;

	assign oOE_N = oe_n;
	assign oRD_N = rd_n;
	assign oWR_N = wr_n;

endmodule 