#pragma once

void writeLine(spi_device_handle_t spi, int16_t xbegin, int16_t ybegin, int16_t xend, 
         int16_t yend, uint16_t color);

void drawLine(spi_device_handle_t spi, int16_t xbegin, int16_t ybegin, int16_t xend, 
        int16_t yend, uint16_t color);

void drawVLine(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, 
        uint16_t length, uint16_t color);

void drawHLine(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin,
       uint16_t length, uint16_t color);

void drawRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t color);

void fillRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t color);

void drawTriangle(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, 
         uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void fillTriangle(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, 
         uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

bool drawPixel(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t color);
