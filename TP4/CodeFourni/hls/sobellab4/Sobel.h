#pragma once

#include <inttypes.h>


#define IMG_WIDTH 1920
#define IMG_HEIGHT 1080
#define IMG_SIZE (IMG_WIDTH * IMG_HEIGHT)

uint8_t sobel_operator(const int fullIndex, uint8_t image[IMG_HEIGHT * IMG_WIDTH]);
void sobel_filter(uint8_t inter_pix[IMG_WIDTH * IMG_HEIGHT], unsigned out_pix[IMG_WIDTH * IMG_HEIGHT]);
