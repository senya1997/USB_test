quit -sim

project::compileall

vsim -L altera_mf_ver -L altera_mf -L lpm_ver -L lpm test_tb -novopt
do C:/work/win/usb_test/verilog/tb/wave/wave.do

transcript on
configure wave -timelineunits us