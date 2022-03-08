#pragma once

void drawRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t color);

void fillRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t color);

void drawTriangle(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, 
         uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void fillTriangle(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, 
         uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void drawCircle(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, 
        uint16_t radius, uint16_t color);

void fillCircleHelper(spi_device_handle_t spi, int16_t x0, int16_t y0, int16_t r, 
        uint8_t corners, int16_t delta, uint16_t color) ;

void fillCircle(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, 
        uint16_t radius, uint16_t color);

void drawRoundRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t radius, uint16_t color);

void fillRoundRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t radius, uint16_t color);
        
