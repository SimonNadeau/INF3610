############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2018 Xilinx, Inc. All Rights Reserved.
############################################################
open_project sobellab4
set_top sobel_filter
add_files SobelLab4/Sobel.cpp
add_files -tb sobellab4/sobel_test.cpp
add_files -tb sobellab4/ap_bmp.cpp
open_solution "solution1"
set_part {xc7z020clg484-1} -tool vivado
create_clock -period 10 -name default
#source "./sobellab4/solution1/directives.tcl"
csim_design -O -compiler gcc
csynth_design
cosim_design
export_design -rtl verilog -format ip_catalog -description "Sobel" -vendor "polymtl.ca" -display_name "Sobel"
