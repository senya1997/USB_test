`include "defines.v"

module test_top (
	input iCLK,
	input iRESET,
	
// FTDI600/601 interface (all signals active low):
	input iUSB_CLK,
	
	inout [31 : 0] ioDATA,
	inout [3 : 0] ioBE, // byte enable

	input iTXE_N, // transmit FIFO buffer empty
	input iRXF_N,
	
	output oOE_N,
	output oRD_N,
	output oWR_N,
	
	output [1 : 0] oGPIO // configure FTDI600
);

reg [3 : 0] state;

reg txe_n;
reg rxf_b;

reg tran_st;

reg oe_n;
reg rd_n;
reg wr_n;

localparam S_ILDE = 4'd0;

localparam S_WR_1 = 4'd1;
localparam S_WR_2 = 4'd2;

localparam S_RD_1 = 4'd4;
localparam S_RD_2 = 4'd5;
localparam S_RD_3 = 4'd6;
localparam S_RD_4 = 4'd7;

localparam S_END = 4'd8;

wire TRAN_ST = ~txe_n & iSTART;
wire TRAN_END = (addr == 10'd1023)

	wire CLK_100;

// FTDI600/601:
	wire [31 : 0]	FTDI_DATA;
	wire [3 : 0]	FTDI_BE;

	wire OE_N;
	wire RD_N;
	wire WR_N;

	wire [1 : 0] GPIO;

	wire [31 : 0]	TRAN_DATA;
	wire [9 : 0]	TRAN_ADDR;

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) txe_n <= 1'b1;
	else txe_n <= iTXE_N;
end

always@(posedge iUSB_CLK or negedge iRESET)begin    
	if(reset) 
		begin
			usb_be_out  <= 2'b11;
			usb_oe <= '0;
			usb_rd <= '0;
			usb_wr <= '0;
			flag <= '0;
			data_flag <= '0;
			clr_abort_pipe <= 1'b0;
			state <= IDLE;
		end 
	else
		case (state)
			S_IDLE:   
				begin
					if(usb_txe && (!data_empty))
						begin
						  state <= WR1;
						  flag <= 1;
						end    
					else if (usb_rxf && (!com_fifo_full)) state <= RD1;
					else state <= IDLE;
				end
				       
			S_WR_1:    
				begin
					if(data_flag)
						begin //данные уже подготовлены
							data_rq <= 1'b0;
							data_flag <= 1'b0;
						end
					else data_rq <= 1'b1;
					
					state <= WR2;
				end                                  
			S_WR_2:    
				begin
					if(usb_txe && (!data_empty))
						begin //запрос txe и данные
							usb_wr <= 1'b1;
							data_rq <= 1'b1;
							state <= WR3;        
						end
					else if(usb_txe && data_empty)
						begin//запрос txe есть, данные закончились
							usb_wr <= 1'b0;
							data_rq <= 1'b0;
							data_flag <= 1'b0;

							state <= WR4;       
						end
					else
						begin // Случай, когда FTDI снимает usb_txe
							usb_wr <= 1'b0;
							data_rq <= 1'b0;
							data_flag <= 1'b1;
							state <= WR4; 
						end
				end
			
			
			//Чтение команд из USB       
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
			
			S_END: state <= IDLE;
			
			default: state <= IDLE;
		endcase
end

test_fifo FIFO_BUF_IN(
	.aclr(iRESET),
	
	.rdclk(CLK_100),
	.rdreq(),
	.q(),
	
	.wrclk(iUSB_CLK),
	.wrreq(),
	.data(),
	
	.rdempty(),
	.wrfull()
);

test_fifo FIFO_BUF_OUT(
	.aclr(iRESET),
	
	.rdclk(),
	.rdreq(),
	.q(),
	
	.wrclk(),
	.wrreq(),
	.data(),
	
	.rdempty(),
	.wrfull()
);

test_pll PLL_100(
	.inclk0(iCLK),
	.areset(iRESET),
	
	.c0(CLK_100)
);
	
assign ioDATA = FTDI_DATA;
assign ioBE = FTDI_BE;

assign oOE_N = oe_n;
assign oRD_N = rd_n;
assign oWR_N = wr_n;

assign oGPIO = 2'b00; // '2'b00' - 1 channel, 245 Synchronous FIFO mode

endmodule 