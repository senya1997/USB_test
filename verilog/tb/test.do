quit -sim

project::compileall

vsim -L altera_mf_ver -L altera_mf test_tb -novopt
do ../../test/tb/wave/wave.do

transcript on
configure wave -timelineunits us