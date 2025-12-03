set modes {}

foreach arg $argv {
    switch -- $arg {
        csim    { lappend modes csim }
        csynth  { lappend modes csynth }
        cosim   { lappend modes cosim }
        all     { set modes {csim csynth cosim} }
        default {
            puts "Unknown option: $arg"
            puts "Usage:"
            puts "  vitis_hls -f mm2s_script.tcl -tclargs csim|csynth|cosim|all"
            exit 1
        }
    }
}

if {[llength $modes] == 0} {
    puts "No mode selected."
    puts "Usage:"
    puts "  vitis_hls -f mm2s_script.tcl -tclargs csim|csynth|cosim|all"
    exit 1
}

open_project "proj"
set_top mm2s

add_files src/mm2s.cpp

open_solution -flow_target vitis "solution"
set_part {xcvc1902-vsvd1760-2MP-e-S}
create_clock -period 2.777 

if {[lsearch -exact $modes csim] != -1} {
    puts "=== Running CSIM ==="
    add_files -tb mm2s_tb_csim.cc
    set f0 [open "aiesim/data0.txt" "w"]
    close $f0
    set f1 [open "aiesim/data1.txt" "w"]
    close $f1
    csim_design -clean
}

if {[lsearch -exact $modes csynth] != -1} {
    puts "=== Running CSYNTH ==="
    csynth_design -dump_post_cfg
}

if {[lsearch -exact $modes cosim] != -1} {
    puts "=== Running COSIM ==="
    add_files -tb mm2s_tb_cosim.cc
    cosim_design -tool xsim -rtl verilog -wave_debug -trace_level port
}

if {[lsearch -exact $modes csynth] != -1 || [lsearch -exact $modes cosim] != -1} {
    export_design -format xo
}

close_project
exit