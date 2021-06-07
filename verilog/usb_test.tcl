
puts "\n============================================================================\n"
puts "Auto update date and version of firmware and compile all revisions"
puts "Copy '*.do' script files in modelsim directory"

foreach cur_rev [get_project_revisions] {
	set_current_revision $cur_rev
	
	puts "\n\tCurrent revision: $cur_rev"
	puts "\t\tCompile..." 
	
	execute_flow -compile;
}

puts "\n============================================================================\n"
