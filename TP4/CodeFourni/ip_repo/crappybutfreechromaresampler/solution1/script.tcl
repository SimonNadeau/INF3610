############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
############################################################
open_project CrappyButFreeChromaResampler
set_top CrappyButFreeChromaResampler
add_files CrappyButFreeChromaResampler/CrappyButFreeChromaResampler.cpp
open_solution "solution1"
set_part {xc7z020clg484-1} -tool vivado
create_clock -period 6 -name default
#source "./CrappyButFreeChromaResampler/solution1/directives.tcl"
#csim_design -compiler gcc
csynth_design
#cosim_design
export_design -rtl verilog -format ip_catalog -description "Because the Xilinx one isn't free" -vendor "polymtl.ca" -display_name "Crappy But Free Chroma Resampler"
