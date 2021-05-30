## Generated SDC file "usb_test.out.sdc"

## Copyright (C) 2018  Intel Corporation. All rights reserved.
## Your use of Intel Corporation's design tools, logic functions 
## and other software and tools, and its AMPP partner logic 
## functions, and any output files from any of the foregoing 
## (including device programming or simulation files), and any 
## associated documentation or information are expressly subject 
## to the terms and conditions of the Intel Program License 
## Subscription Agreement, the Intel Quartus Prime License Agreement,
## the Intel FPGA IP License Agreement, or other applicable license
## agreement, including, without limitation, that your use is for
## the sole purpose of programming logic devices manufactured by
## Intel and sold by Intel or its authorized distributors.  Please
## refer to the applicable agreement for further details.


## VENDOR  "Altera"
## PROGRAM "Quartus Prime"
## VERSION "Version 18.1.0 Build 625 09/12/2018 SJ Lite Edition"

## DATE    "Thu May 27 11:42:39 2021"

##
## DEVICE  "10CL080YF780I7G"
##


#**************************************************************
# Time Information
#**************************************************************

set_time_format -unit ns -decimal_places 3



#**************************************************************
# Create Clock
#**************************************************************

create_clock -name {iCLK} -period 20.000 -waveform { 0.000 10.000 } [get_ports {iCLK}]
create_clock -name {iUSB_CLK} -period 15.160 -waveform { 0.000 7.580 } [get_ports {iUSB_CLK}]


#**************************************************************
# Create Generated Clock
#**************************************************************

create_generated_clock -name {PLL_100|altpll_component|auto_generated|pll1|clk[0]} -source [get_pins {PLL_100|altpll_component|auto_generated|pll1|inclk[0]}] -duty_cycle 50/1 -multiply_by 2 -master_clock {iCLK} [get_pins {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] 


#**************************************************************
# Set Clock Latency
#**************************************************************



#**************************************************************
# Set Clock Uncertainty
#**************************************************************

set_clock_uncertainty -rise_from [get_clocks {iUSB_CLK}] -rise_to [get_clocks {iUSB_CLK}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {iUSB_CLK}] -fall_to [get_clocks {iUSB_CLK}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {iUSB_CLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.080  
set_clock_uncertainty -rise_from [get_clocks {iUSB_CLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.110  
set_clock_uncertainty -rise_from [get_clocks {iUSB_CLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.080  
set_clock_uncertainty -rise_from [get_clocks {iUSB_CLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.110  
set_clock_uncertainty -fall_from [get_clocks {iUSB_CLK}] -rise_to [get_clocks {iUSB_CLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {iUSB_CLK}] -fall_to [get_clocks {iUSB_CLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {iUSB_CLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.080  
set_clock_uncertainty -fall_from [get_clocks {iUSB_CLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.110  
set_clock_uncertainty -fall_from [get_clocks {iUSB_CLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.080  
set_clock_uncertainty -fall_from [get_clocks {iUSB_CLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.110  
set_clock_uncertainty -rise_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -rise_to [get_clocks {iUSB_CLK}] -setup 0.110  
set_clock_uncertainty -rise_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -rise_to [get_clocks {iUSB_CLK}] -hold 0.080  
set_clock_uncertainty -rise_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -fall_to [get_clocks {iUSB_CLK}] -setup 0.110  
set_clock_uncertainty -rise_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -fall_to [get_clocks {iUSB_CLK}] -hold 0.080  
set_clock_uncertainty -rise_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -rise_to [get_clocks {iUSB_CLK}] -setup 0.110  
set_clock_uncertainty -fall_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -rise_to [get_clocks {iUSB_CLK}] -hold 0.080  
set_clock_uncertainty -fall_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -fall_to [get_clocks {iUSB_CLK}] -setup 0.110  
set_clock_uncertainty -fall_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -fall_to [get_clocks {iUSB_CLK}] -hold 0.080  
set_clock_uncertainty -fall_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -rise_to [get_clocks {iUSB_CLK}]  0.040  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -fall_to [get_clocks {iUSB_CLK}]  0.040  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.070  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.100  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.070  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.100  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -rise_to [get_clocks {iCLK}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {iCLK}] -fall_to [get_clocks {iCLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -rise_to [get_clocks {iUSB_CLK}]  0.040  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -fall_to [get_clocks {iUSB_CLK}]  0.040  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.070  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -rise_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.100  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.070  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -fall_to [get_clocks {PLL_100|altpll_component|auto_generated|pll1|clk[0]}] -hold 0.100  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -rise_to [get_clocks {iCLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {iCLK}] -fall_to [get_clocks {iCLK}]  0.020  


#**************************************************************
# Set Input Delay
#**************************************************************



#**************************************************************
# Set Output Delay
#**************************************************************



#**************************************************************
# Set Clock Groups
#**************************************************************



#**************************************************************
# Set False Path
#**************************************************************

set_false_path -from [get_keepers {*rdptr_g*}] -to [get_keepers {*ws_dgrp|dffpipe_re9:dffpipe15|dffe16a*}]
set_false_path -from [get_keepers {*delayed_wrptr_g*}] -to [get_keepers {*rs_dgwp|dffpipe_qe9:dffpipe12|dffe13a*}]


#**************************************************************
# Set Multicycle Path
#**************************************************************



#**************************************************************
# Set Maximum Delay
#**************************************************************



#**************************************************************
# Set Minimum Delay
#**************************************************************



#**************************************************************
# Set Input Transition
#**************************************************************

