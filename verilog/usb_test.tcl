set cur_rev [get_current_revision]

post_message "\n***************************************************************"
post_message "Before build: start auto update date and version of firmware"
post_message "Copy '*.do' script files in modelsim directory"
post_message "Current revision: $cur_rev"

# DEBUG:
	set arg0 [lindex $quartus(args) 0]
	set arg1 [lindex $quartus(args) 1]
	set arg2 [lindex $quartus(args) 2]
	
	post_message [format "Arguments: %s %s %s" $arg0 $arg1 $arg2]

# name of define which turn off part of RTL:
	set name_def_date	FW_DATE
	set name_def_ver	FW_VER
		
# path of elements:
	set path_modelsim ./../../../fpga/modelsim/usb_test/
	
	set path_sim_start	./tb/test.do
	set path_wave			./tb/wave/wave.do

	set path_def ./defines.v

# get date and format it:
	set sys_time_sec [clock seconds]
	set date [clock format $sys_time_sec -format {%m_%d_%y}]

# check for exist entered define names:
	set f_def [open $path_def r+]
	set flag_def_exist 0
	
while {[gets $f_def temp_str] >= 0} {
	set ind_def [string first $name_def_ver $temp_str]
	
	if {$ind_def != -1} {
		incr flag_def_exist
		
		set ind_build_num [expr {$ind_def + [string length $name_def_ver]}]
		set temp_str_length [string length $temp_str]
		
		set build_num [string range $temp_str $ind_build_num $temp_str_length]
		
		incr build_num
		
		seek $f_def -$temp_str_length current
		puts $f_def "`define $name_def_ver $build_num"
		
		break
	}
}

if {$flag_def_exist == 0} {
	post_message -type error "\n'$name_def_ver' define was not found in $path_def"
	return -code error "\n'$name_def_ver' define was not found in $path_def"
}

close $f_def

# copy scripts and check paths:
foreach path_script [list $path_sim_start $path_wave] {
	catch {file copy -force $path_script	$path_modelsim} res

	if {$res == 0} {
		post_message -type warning "Failure to copy script '$path_script' in path '$path_modelsim', $res"
	} else {
		post_message "Succsess to copy script '$path_script' in path '$path_modelsim'"
	}
}
