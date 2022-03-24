#pragma once
#include "lcdfont.h"

#define ascii_zero 48
#define letter_spacing 3

inline uint8_t numtoascii(uint8_t number){
    return number+ascii_zero;
}

inline uint8_t asciitonum(uint8_t number){
    return number-ascii_zero;
}


void drawChar(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, char character, 
        struct lcdfont font);

void drawText(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, const char *string,
        struct lcdfont font);