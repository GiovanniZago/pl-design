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
add_files -tb "mm2s_tb.cc tb_utils.cc"

if {[lsearch -exact $modes csim] != -1} {
    open_solution -flow_target vitis "solution"
    set_part {xcvc1902-vsvd1760-2MP-e-S}
    create_clock -period 2.777 

    puts "=== Running CSIM ==="
    set f0 [open "aiesim/data0.txt" "w"]
    close $f0
    set f1 [open "aiesim/data1.txt" "w"]
    close $f1
    csim_design -clean
    close_solution
}

if {[lsearch -exact $modes csynth] != -1} {
    open_solution -flow_target vitis -reset "solution"
    set_part {xcvc1902-vsvd1760-2MP-e-S}
    create_clock -period 2.777 

    puts "=== Running CSYNTH ==="
    csynth_design -dump_post_cfg

    puts "=== Exporting XO ==="
    export_design -format xo
    close_solution
}

if {[lsearch -exact $modes cosim] != -1} {
    open_solution -flow_target vitis "solution"
    set_part {xcvc1902-vsvd1760-2MP-e-S}
    create_clock -period 2.777 

    set sol_dir [get_solution -directory]

    # Define one or more files that must exist after csynth
    set csynth_rpt   [file join $sol_dir "syn" "report" "csynth.rpt"]
    set rtl_verilog  [file join $sol_dir "syn" "verilog" "mm2s.v"]  ;# adapt top name

    # require the csynth report
    if {![file exists $csynth_rpt]} {
        puts "ERROR: csynth_design has not been run for the current solution."
        puts "       Run csynth before cosim (e.g. with -tclargs csynth cosim or all)."
        exit 1
    }

    # require RTL file
    if {![file exists $rtl_verilog]} {
        puts "ERROR: RTL Verilog file $rtl_verilog not found. Synthesis incomplete."
        exit 1
    }
    
    puts "=== Running COSIM ==="
    set f0 [open "aiesim/data0.txt" "w"]
    close $f0
    set f1 [open "aiesim/data1.txt" "w"]
    close $f1
    set f0 [open "aiesim/data0_split.txt" "w"]
    close $f0
    set f1 [open "aiesim/data1_split.txt" "w"]
    close $f1
    cosim_design -tool xsim -rtl verilog -trace_level all
    close_solution
}

close_project
exit