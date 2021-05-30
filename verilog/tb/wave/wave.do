onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /test_tb/DUT/reset
add wave -noupdate /test_tb/DUT/iCLK
add wave -noupdate /test_tb/DUT/CLK_100
add wave -noupdate /test_tb/DUT/iUSB_CLK
add wave -noupdate -radix unsigned /test_tb/FTDI_IMIT/data_in_cnt
add wave -noupdate -radix unsigned /test_tb/FTDI_IMIT/data_out_cnt
add wave -noupdate /test_tb/FTDI_IMIT/data_direct
add wave -noupdate -expand -group ports -radix hexadecimal /test_tb/DUT/ioDATA
add wave -noupdate -expand -group ports /test_tb/DUT/ioBE
add wave -noupdate -expand -group ports -color Magenta /test_tb/DUT/iTXE_N
add wave -noupdate -expand -group ports -color Magenta /test_tb/DUT/iRXF_N
add wave -noupdate -expand -group ports -color Yellow /test_tb/DUT/oOE_N
add wave -noupdate -expand -group ports -color Yellow /test_tb/DUT/oRD_N
add wave -noupdate -expand -group ports -color Yellow /test_tb/DUT/oWR_N
add wave -noupdate -color {Slate Blue} -radix unsigned /test_tb/DUT/state
add wave -noupdate /test_tb/DUT/shift_txe_n
add wave -noupdate /test_tb/DUT/shift_rxf_n
add wave -noupdate /test_tb/DUT/buf_out_loaded
add wave -noupdate /test_tb/DUT/buf_out_loaded_clk100
add wave -noupdate -expand -group flags /test_tb/DUT/flag_upd_fw_n
add wave -noupdate -expand -group flags /test_tb/DUT/flag_upd_buf_out
add wave -noupdate -expand -group flags /test_tb/DUT/flag_send_buf_out
add wave -noupdate -expand -group flags /test_tb/DUT/flag_buf_in_loaded
add wave -noupdate -expand -group flags /test_tb/DUT/flag_buf_in_loaded_d
add wave -noupdate -expand -group flags /test_tb/DUT/flag_buf_in_loaded_clk100
add wave -noupdate -expand -group flags /test_tb/DUT/flag_buf_out_loaded
add wave -noupdate -expand -group BUF_OUT -radix hexadecimal /test_tb/DUT/buf_out_data
add wave -noupdate -expand -group BUF_OUT -radix unsigned /test_tb/DUT/buf_out_num_data
add wave -noupdate -expand -group BUF_OUT -radix unsigned /test_tb/DUT/buf_out_cnt_data
add wave -noupdate -expand -group BUF_OUT /test_tb/DUT/buf_out_wr
add wave -noupdate -expand -group BUF_OUT /test_tb/DUT/buf_out_rd
add wave -noupdate -expand -group BUF_OUT /test_tb/DUT/BUF_OUT_LOADED
add wave -noupdate -expand -group {FIFO OUT} /test_tb/DUT/FIFO_BUF_OUT/aclr
add wave -noupdate -expand -group {FIFO OUT} -radix hexadecimal /test_tb/DUT/FIFO_BUF_OUT/data
add wave -noupdate -expand -group {FIFO OUT} -color {Slate Blue} /test_tb/DUT/FIFO_BUF_OUT/wrclk
add wave -noupdate -expand -group {FIFO OUT} -color {Slate Blue} /test_tb/DUT/FIFO_BUF_OUT/wrreq
add wave -noupdate -expand -group {FIFO OUT} -color {Medium Orchid} /test_tb/DUT/FIFO_BUF_OUT/rdclk
add wave -noupdate -expand -group {FIFO OUT} -color {Medium Orchid} /test_tb/DUT/FIFO_BUF_OUT/rdreq
add wave -noupdate -expand -group {FIFO OUT} /test_tb/DUT/FIFO_BUF_OUT/q
add wave -noupdate -expand -group {FIFO OUT} /test_tb/DUT/FIFO_BUF_OUT/rdempty
add wave -noupdate -expand -group {FIFO OUT} /test_tb/DUT/FIFO_BUF_OUT/wrfull
add wave -noupdate -expand -group {FIFO IN} -radix hexadecimal /test_tb/DUT/FIFO_BUF_IN/data
add wave -noupdate -expand -group {FIFO IN} -color {Slate Blue} /test_tb/DUT/FIFO_BUF_IN/wrclk
add wave -noupdate -expand -group {FIFO IN} -color {Slate Blue} /test_tb/DUT/FIFO_BUF_IN/wrreq
add wave -noupdate -expand -group {FIFO IN} -color {Medium Orchid} /test_tb/DUT/FIFO_BUF_IN/rdclk
add wave -noupdate -expand -group {FIFO IN} -color {Medium Orchid} /test_tb/DUT/FIFO_BUF_IN/rdreq
add wave -noupdate -expand -group {FIFO IN} -radix hexadecimal /test_tb/DUT/FIFO_BUF_IN/q
add wave -noupdate -expand -group {FIFO IN} /test_tb/DUT/FIFO_BUF_IN/rdempty
add wave -noupdate -expand -group {FIFO IN} /test_tb/DUT/FIFO_BUF_IN/wrfull
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1025]}
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1024]}
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1023]}
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1022]}
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1021]}
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[2]}
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1]}
add wave -noupdate -group {mem FIFO OUT} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_OUT/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[0]}
add wave -noupdate -group {mem FIFO IN} {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1025]}
add wave -noupdate -group {mem FIFO IN} {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1024]}
add wave -noupdate -group {mem FIFO IN} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1023]}
add wave -noupdate -group {mem FIFO IN} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1022]}
add wave -noupdate -group {mem FIFO IN} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1021]}
add wave -noupdate -group {mem FIFO IN} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[2]}
add wave -noupdate -group {mem FIFO IN} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[1]}
add wave -noupdate -group {mem FIFO IN} -radix hexadecimal {/test_tb/DUT/FIFO_BUF_IN/dcfifo_component/DCFIFO_MW/LOWLATENCY/mem_data[0]}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {36414548 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 242
configure wave -valuecolwidth 69
configure wave -justifyvalue left
configure wave -signalnamewidth 2
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits us
update
WaveRestoreZoom {36258649 ps} {36641890 ps}
