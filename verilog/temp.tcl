set rev_name [get_current_revision]

post_message "Current revision: $rev_name"

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
		
		if {$ind_def != -1} {
			set build_num [string range $temp_str $ind_build_num [string length $temp_str]]
			
			incr build_num
			
			puts $f_def "\t\t`define $name_def_ver $build_num"
		} else {
			return -code error "'$name_def_ver' define was found but format is wrong"
		}
		
		break
	}
}

if {$flag_def_exist == 0} {
	return -code error "'$name_def_ver' define was not found in $path_def"
}

close $f_def

# copy scripts and check paths:
	foreach path_script [$path_sim_start $path_wave] {
		if{[catch {file copy -force $path_script	$path_modelsim} res]}{
			post_message -type warning "Failure to copy script '$path_script' in path '$path_modelsim', $res"
		} else {
			post_message "Succsess to copy script '$path_script' in path '$path_modelsim'"
		}
	}
