#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_ili9486_base.h"
#include "lcd_ili9486_primitives.h"


void writeLine(spi_device_handle_t spi, int16_t xbegin, int16_t ybegin, int16_t xend, 
         int16_t yend, uint16_t color)
{
    bool steep = abs(ybegin-yend) > abs(xbegin-xend);
    if(steep){
        _swap_int16_t(xbegin, ybegin);
        _swap_int16_t(xend, yend);
    }
        
    if(xbegin > xend){
        _swap_int16_t(xbegin, xend);
        _swap_int16_t(ybegin, yend);
    }
        
    int16_t dx, dy, ystep, err;
    dx = xend - xbegin;
    dy = abs(yend - ybegin);
    err = dx / 2;

    if (ybegin < yend) {
     ystep = 1;
    } else {
     ystep = -1;
    }

    for (; xbegin <= xend; xbegin++) {
      if (steep) {
        drawPixel(spi, ybegin, xbegin, color);
      } else {
        drawPixel(spi, xbegin, ybegin, color);
      }
      err -= dy;
      if (err < 0) {
        ybegin += ystep;
        err += dx;
      }
    }
}

void drawLine(spi_device_handle_t spi, int16_t xbegin, int16_t ybegin, int16_t xend, 
        int16_t yend, uint16_t color)
{
    if(xbegin == xend){
        uint16_t length = abs(yend-ybegin);
        ybegin = ybegin > yend ? yend : ybegin;
        drawVLine(spi, xbegin, ybegin, length, color);
    }
    else if(ybegin == yend){
        uint16_t length = abs(yend-ybegin);
        xbegin = xbegin > yend ? xend : xbegin;
        drawHLine(spi, xbegin, ybegin, length, color);
    }
    else{
        writeLine(spi, xbegin, ybegin, xend, yend, color);
    }
}

void drawVLine(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, 
        uint16_t length, uint16_t color)
{
    uint16_t* linebuffer = NULL;
    setWriteArea(spi, xbegin, ybegin, 1, length);
    esp_err_t ret;
    static spi_transaction_t trans;
    linebuffer= heap_caps_malloc(2*length, MALLOC_CAP_DMA);
    for(int i =0; i<length; i++){
        linebuffer[i]=color;
    }
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.tx_buffer=linebuffer;       //finally send the line data
    trans.length=16*length;          //Data length, in bits
    trans.user =(void*)1;
    trans.flags=0; //undo SPI_TRANS_USE_TXDATA flag
    ret=spi_device_queue_trans(spi, &trans, portMAX_DELAY);
    assert(ret==ESP_OK);
    spi_transaction_t *rtrans;
    ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
    assert(ret==ESP_OK);
    heap_caps_free(linebuffer);
}


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
    uint16_t* linebuffer = NULL;
    esp_err_t ret;
    static spi_transaction_t trans;
    heap_caps_free(linebuffer);
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

bool drawPixel(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t color) 
{
    if (x >= 320|| y >= 480) {
        return false;
    }
    esp_err_t ret;
    static spi_transaction_t trans[6];
    for (int i=0; i<6; i++) {
      memset(&trans[i], 0, sizeof(spi_transaction_t));
      trans[i].flags = SPI_TRANS_USE_TXDATA;
      if((i+1) % 2){
          trans[i].user = (void*)0;
      }
      else{
          trans[i].user = (void*)1;
      }
    }
    trans[0].tx_data[0]=0x00;
    trans[0].tx_data[1]=CMD_CASET;
    trans[0].length = 16;
    trans[1].tx_data[0]=0x00;
    trans[1].tx_data[1]=(x>>8);
    trans[1].tx_data[2]= 0x00;
    trans[1].tx_data[3]=(x&0xFF);
    trans[1].length = 32;
    trans[2].tx_data[0]= 0x00;
    trans[2].tx_data[1]= CMD_PASET;
    trans[2].length = 16;
    trans[3].tx_data[0] = 0x00;
    trans[3].tx_data[1] = (y>>8);
    trans[3].tx_data[2] = 0x00;
    trans[3].tx_data[3] = (y&0xFF);
    trans[3].length = 32;
    trans[4].tx_data[0] = 0x00;
    trans[4].tx_data[1] = CMD_RAMWR;
    trans[4].length = 16;
    trans[5].tx_data[0] = (uint16_t)(color & 0x00FF);
    trans[5].tx_data[1] = color >> 8;
    trans[5].length=16;
    trans[5].flags = SPI_TRANS_USE_TXDATA;
    trans[5].user = (void *)1;
    ret=spi_device_queue_trans(spi, &trans[0], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[1], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[1], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[2], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[3], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[3], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[4], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[5], portMAX_DELAY);
    assert(ret==ESP_OK);
    return true;
}



void drawHLine(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin,
        uint16_t length, uint16_t color){
            
    uint16_t* linebuffer = NULL;
    setWriteArea(spi, xbegin, ybegin, length, 1);
    esp_err_t ret;
    static spi_transaction_t trans;
    linebuffer= heap_caps_malloc(2*length, MALLOC_CAP_DMA);
    for(int i =0; i<length; i++){
        linebuffer[i]=color;
    }
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.tx_buffer=linebuffer;       //finally send the line data
    trans.length=16*length;          //Data length, in bits
    trans.user =(void*)1;
    trans.flags=0; //undo SPI_TRANS_USE_TXDATA flag
    ret=spi_device_queue_trans(spi, &trans, portMAX_DELAY);
    assert(ret==ESP_OK);
    spi_transaction_t *rtrans;

    ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
    assert(ret==ESP_OK);
    
    heap_caps_free(linebuffer);
}
