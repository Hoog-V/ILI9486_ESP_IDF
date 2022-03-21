#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <esp_task_wdt.h>
#include "lcd_ili9486_base.h"
#include "lcd_ili9486_primitives.h"


void drawRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t color)
{
    drawVLine(spi, xpos, ypos, height, color);
    drawVLine(spi, (xpos+width), ypos, height, color);
    drawHLine(spi, xpos, ypos, width, color);
    drawHLine(spi, xpos, (ypos+height), width, color);
}

void fillRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t color)
{
    static uint16_t* linebuffer = NULL;
    esp_err_t ret;
    static spi_transaction_t trans;
    linebuffer= heap_caps_malloc(2*width, MALLOC_CAP_DMA);
    setWriteArea(spi, xpos, ypos, width, height);
    for(int i =0; i<width; i++){
        linebuffer[i]=color;
    }
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.tx_buffer=linebuffer;       //finally send the line data
    trans.length=16*width;          //Data length, in bits
    trans.user =(void*)1;
    trans.flags=0; //undo SPI_TRANS_USE_TXDATA flag
    spi_transaction_t *rtrans;
    for(int i = 0; i < height; i++){
        ret=spi_device_queue_trans(spi, &trans, portMAX_DELAY);
        assert(ret==ESP_OK);
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        }
    heap_caps_free(linebuffer);
}


void drawTriangle(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, 
         uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    drawLine(spi, x0, y0, x1, y1, color);
    drawLine(spi, x1, y1, x2, y2, color);
    drawLine(spi, x2, y2, x0, y0, color);
}

void fillTriangle(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, 
         uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
  
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }
  if (y1 > y2) {
    _swap_int16_t(y2, y1);
    _swap_int16_t(x2, x1);
  }
  if (y0 > y1) {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }

  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    drawHLine(spi, a, y0, b - a + 1, color);
    return;
  }

  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
          dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
    last = y1; // Include y1 scanline
  else
    last = y1 - 1; // Skip it

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      _swap_int16_t(a, b);
    drawHLine(spi, a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = (int32_t)dx12 * (y - y1);
  sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      _swap_int16_t(a, b);
    drawHLine(spi, a, y, b - a + 1, color);
  }
}

void drawCircle(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, 
        uint16_t radius, uint16_t color)
{
  int16_t f = 1 - radius;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * radius;
  int16_t x = 0;
  int16_t y = radius;

  drawPixel(spi, xbegin, ybegin + radius, color);
  drawPixel(spi, xbegin, ybegin - radius, color);
  drawPixel(spi, xbegin + radius, ybegin, color);
  drawPixel(spi, xbegin - radius, ybegin, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(spi, xbegin + x, ybegin + y, color);
    drawPixel(spi, xbegin - x, ybegin + y, color);
    drawPixel(spi, xbegin + x, ybegin - y, color);
    drawPixel(spi, xbegin - x, ybegin - y, color);
    drawPixel(spi, xbegin + y, ybegin + x, color);
    drawPixel(spi, xbegin - y, ybegin + x, color);
    drawPixel(spi, xbegin + y, ybegin - x, color);
    drawPixel(spi, xbegin - y, ybegin - x, color);
}
}

void fillCircleHelper(spi_device_handle_t spi, int16_t x0, int16_t y0, int16_t r, 
        uint8_t corners, int16_t delta, uint16_t color) 
{

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;
  int16_t px = x;
  int16_t py = y;

  delta++; // Avoid some +1's in the loop

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    // These checks avoid double-drawing certain lines, important
    // for the SSD1306 library which has an INVERT drawing mode.
    if (x < (y + 1)) {
      if (corners & 1)
        drawVLine(spi, x0 + x, y0 - y, 2 * y + delta, color);
      if (corners & 2)
        drawVLine(spi, x0 - x, y0 - y, 2 * y + delta, color);
    }
    if (y != py) {
      if (corners & 1)
        drawVLine(spi, x0 + py, y0 - px, 2 * px + delta, color);
      if (corners & 2)
        drawVLine(spi, x0 - py, y0 - px, 2 * px + delta, color);
      py = y;
    }
    px = x;
  }
}

void fillCircle(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, 
        uint16_t radius, uint16_t color)
{
    drawVLine(spi, xbegin, ybegin - radius, 2 * radius + 1, color);
    fillCircleHelper(spi, xbegin, ybegin, radius, 3, 0, color);
}


void drawCircleHelper(spi_device_handle_t spi, int16_t x0, int16_t y0, int16_t r,
         uint8_t cornername, uint16_t color) 
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(spi, x0 + x, y0 + y, color);
      drawPixel(spi, x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      drawPixel(spi, x0 + x, y0 - y, color);
      drawPixel(spi, x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(spi, x0 - y, y0 + x, color);
      drawPixel(spi, x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(spi, x0 - y, y0 - x, color);
      drawPixel(spi, x0 - x, y0 - y, color);
    }
  }
}



void drawRoundRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t radius, uint16_t color)
{
  int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
  if (radius > max_radius)
    radius = max_radius;
  // smarter version
  drawHLine(spi, xpos + radius, ypos, width - 2 * radius, color);         // Top
  drawHLine(spi, xpos + radius, ypos + height - 1, width - 2 * radius, color); // Bottom
  drawVLine(spi, xpos, ypos + radius, height - 2 * radius, color);         // Left
  drawVLine(spi, xpos + width - 1, ypos + radius, height - 2 * radius, color); // Right
  // draw four corners
  drawCircleHelper(spi, xpos + radius, ypos + radius, radius, 1, color);
  drawCircleHelper(spi, xpos + width - radius - 1, ypos + radius, radius, 2, color);
  drawCircleHelper(spi, xpos + width - radius - 1, ypos + height - radius - 1, radius, 
                   4, color);
  drawCircleHelper(spi, xpos + radius, ypos + height - radius - 1, radius, 8, color);
}

void fillRoundRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, 
        uint16_t height, uint16_t radius, uint16_t color)
{  
    int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
    if (radius > max_radius)
        radius = max_radius;
    // smarter version
    fillRect(spi, xpos + radius, ypos, width - 2 * radius, height, color);
    // draw four corners
    fillCircleHelper(spi, xpos + width - radius - 1, ypos + radius, radius, 1, 
        height - 2 * radius - 1, color);
    fillCircleHelper(spi, xpos + radius, ypos + radius, radius, 2, 
        height - 2 * radius - 1, color);
}