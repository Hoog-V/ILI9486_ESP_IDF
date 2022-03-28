#pragma once

#include <stdint.h>

#define BLACK 0x0000
#define WHITE 0xFFFF


inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) 
{
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
    uint16_t conv = ((color & 0x00FF) << 8) | ((color & 0xFF00) >> 8);
    return conv;
}