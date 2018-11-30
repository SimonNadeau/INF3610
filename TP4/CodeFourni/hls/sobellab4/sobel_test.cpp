/*******************************************************************************
Vendor: Xilinx 
Associated Filename: sobel_test.cpp
Purpose: Testbench file for sobel edge detection
Revision History: February 13, 2012 - initial release
                  2015~2017 modifié pour INF3610.
                                                
*******************************************************************************
Â© Copyright 2008 - 2012 Xilinx, Inc. All rights reserved. 

This file contains confidential and proprietary information of Xilinx, Inc. and 
is protected under U.S. and international copyright and other intellectual 
property laws.

DISCLAIMER
This disclaimer is not a license and does not grant any rights to the materials 
distributed herewith. Except as otherwise provided in a valid license issued to 
you by Xilinx, and to the maximum extent permitted by applicable law: 
(1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX 
HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, 
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR 
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether 
in contract or tort, including negligence, or under any other theory of 
liability) for any loss or damage of any kind or nature related to, arising under 
or in connection with these materials, including for any direct, or any indirect, 
special, incidental, or consequential loss or damage (including loss of data, 
profits, goodwill, or any type of loss or damage suffered as a result of any 
action brought by a third party) even if such damage or loss was reasonably 
foreseeable or Xilinx had been advised of the possibility of the same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any 
application requiring fail-safe performance, such as life-support or safety 
devices or systems, Class III medical devices, nuclear facilities, applications 
related to the deployment of airbags, or any other applications that could lead 
to death, personal injury, or severe property or environmental damage 
(individually and collectively, "Critical Applications"). Customer assumes the 
sole risk and liability of any use of Xilinx products in Critical Applications, 
subject only to applicable laws and regulations governing limitations on product 
liability. 

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT 
ALL TIMES.

*******************************************************************************/

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ap_bmp.h"
#include "Sobel.h"

// I/O Image Settings
#define INPUT_IMAGE_BASE	"test_1080p.bmp"
#define OUTPUT_IMAGE_BASE	"result_1080p.bmp"
#define OUTPUT_IMAGE_GOLDEN	"result_1080p_golden.bmp"
#define WORKDIR "../../../"

static unsigned char rgb2y(uint8_t R, uint8_t G, uint8_t B)
{
	return ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
}

int main () {
	int	width, height;
	char *tempbuf = (char *)malloc(2000 * sizeof(char));
	char *tempbuf1 = (char *)malloc(2000 * sizeof(char));
	int check_results = 0;

	// Arrays to store image data
	unsigned char *R;
	unsigned char *G;
	unsigned char *B;
	uint8_t * in_pix = (uint8_t*)malloc(IMG_HEIGHT * IMG_WIDTH * sizeof(unsigned char));
	unsigned * out_pix = (unsigned *) malloc(IMG_HEIGHT * IMG_WIDTH * sizeof(unsigned));


	R = (unsigned char *)malloc(IMG_HEIGHT * IMG_WIDTH * sizeof(unsigned char));
	G = (unsigned char *)malloc(IMG_HEIGHT * IMG_WIDTH * sizeof(unsigned char));
	B = (unsigned char *)malloc(IMG_HEIGHT * IMG_WIDTH * sizeof(unsigned char));

	//Get image data from disk
	// Fill a frame with data
	int read_tmp = BMP_Read(WORKDIR INPUT_IMAGE_BASE, IMG_HEIGHT, IMG_WIDTH, R, G, B);
	if(read_tmp != 0) {
		printf("%s Loading image failed\n", tempbuf);
		exit (1);
	}

	for(int i = 0; i < IMG_HEIGHT * IMG_WIDTH; i++){
		in_pix[i] = rgb2y(R[i], G[i], B[i]);
	}

	// Hardware Function
	sobel_filter(in_pix, out_pix);

	for(int i = 0; i < IMG_HEIGHT * IMG_WIDTH; i++){
		typedef union {
			uint8_t pix[4];
			unsigned full;
		} pix;
		pix mypix;
		mypix.full = out_pix[i];
		R[i] = mypix.pix[1];
		G[i] = mypix.pix[2];
		B[i] = mypix.pix[3];
	}

	//Write the image back to disk
	int write_tmp = BMP_Write(WORKDIR OUTPUT_IMAGE_BASE, IMG_HEIGHT, IMG_WIDTH, R, G, B);
	if(write_tmp != 0) {
		printf("WriteBMP %s failed\n", tempbuf);
		return 0;
		exit(1);
	}
	free(R);
	free(G);
	free(B);
	free(tempbuf);
	free(tempbuf1);
	free(in_pix);
	free(out_pix);
	printf("Simulation Complete\n");
	check_results = system("diff " WORKDIR OUTPUT_IMAGE_GOLDEN " " WORKDIR OUTPUT_IMAGE_BASE);
	if(check_results != 0){
		printf("Output image has mismatches with reference output image!\n");
		check_results = 1;
	} else{
		printf("Output image matches the reference output image\n");
	}
	return check_results;
}
