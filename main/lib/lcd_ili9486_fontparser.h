#pragma once
#include "lcdfont.h"
void drawChar(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, char character, struct lcdfont font);