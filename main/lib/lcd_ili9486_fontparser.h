#pragma once
#include "lcdfont.h"

#define ascii_zero 48

inline uint8_t numtoascii(uint8_t number){
    return number+ascii_zero;
}

inline uint8_t asciitonum(uint8_t number){
    return number-ascii_zero;
}


void drawChar(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart,  
        unsigned char character, uint16_t color, struct lcdfont font, uint8_t scale);

void drawText(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, 
        const char *string, uint16_t color, struct lcdfont font, uint8_t textsize);


void scale_bitmap(const unsigned char *origbuffer, char *newbuffer, uint8_t scale);

void print_bitmap16(unsigned char *buffer, uint8_t w, uint8_t h);