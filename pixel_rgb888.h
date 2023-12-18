#ifndef PIXEL_RGB888_H
#define PIXEL_RGB888_H
#include<stdint.h>
#include<iostream>

struct pixel_rgb888
{
    uint8_t r,g,b;
    pixel_rgb888();
};
void rgb_to_yuv(uint8_t r,uint8_t g,uint8_t b,uint8_t&y ,uint8_t&u,uint8_t&v);
void rgb_to_yuv(pixel_rgb888 p,uint16_t&y ,uint16_t&u,uint16_t&v);
void rgb_to_yuv_avx(pixel_rgb888* pix_line1,pixel_rgb888* pix_line2,uint8_t* y,uint8_t* u,uint8_t* v);

#endif // PIXEL_RGB888_H
