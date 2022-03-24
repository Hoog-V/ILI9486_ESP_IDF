#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_ili9486_base.h"
#include "lcd_ili9486_fontparser.h"
#include "lcdfont.h"




void drawChar(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, char character, 
        struct lcdfont font) 
{
    int x,y;
    int set;
    const char *bitmap = font.char_addr[character-ascii_zero];
    uint8_t character_width = font.width[character-ascii_zero];
    uint8_t a_of_bytes = font.font_size/font.line_size;
    for (x=0; x < character_width; x++) {
    for(int p=0; p< a_of_bytes; p++){
     for (y=0; y < font.line_size; y++) {
        set = bitmap[x+(p*character_width)] & 1 << y;
        if(set){
            drawPixel(spi, xstart+x, ystart+y+(p*8), 0xFFFF);
        }
        }
    }
    }
}

void drawText(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, const char *string,
        struct lcdfont font)
{
    uint16_t x = xstart;
    while(*string){
        drawChar(spi, x, ystart, *string, font);
        x += font.width[ asciitonum(*string) ] + letter_spacing;
        string++;
    }
}
