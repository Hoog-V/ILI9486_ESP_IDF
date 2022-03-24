#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_ili9486_base.h"
#include "lcd_ili9486_fontparser.h"
#include "lcdfont.h"

#define ArrStartaddr 0x20

inline unsigned char getarrOffset(unsigned char character){
    return character-ArrStartaddr;
}


void drawChar(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, 
        unsigned char character,uint16_t color, struct lcdfont font) 
{
    int x,y;
    int set;
    uint8_t character_width;
    
    
    uint8_t a_of_bytes = font.font_size/font.line_size;
    const char *bitmap = font.addr[getarrOffset(character)];
    bool flipped = font.font_flipped;
    if(font.variable_width){
        character_width = font.width[getarrOffset(character)];
    }
    else{
        character_width = *font.width;
    }

    for (x=0; x < character_width; x++) {
        for(int p=0; p< a_of_bytes; p++){
             for (y=0; y < font.line_size; y++) {
                 set = bitmap[x+(p*character_width)] & 1 << y;
                 if(set && flipped){
                     drawPixel(spi, xstart+y, ystart+x+(p*8), color);
                 }
                 else if (set && !flipped)
                 {
                    drawPixel(spi, xstart+x, ystart+y+(p*8), color);
                 }
              }
        }
    }
    

}

void drawText(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, const char *string,
        uint16_t color, struct lcdfont font)
{
    uint16_t x = xstart;
    while(*string){
        if(*string != ' '){
        drawChar(spi, x, ystart,*string, color, font);
        }
        else{
            printf("Found space with width: %d on index %d\r\n", font.width[getarrOffset((unsigned char) *string)], getarrOffset((unsigned char)*string)-32);
        }

        if(font.variable_width){
        x += font.width[getarrOffset((unsigned char)*string)] + font.letter_spacing;
        }
        else{
            x+=*font.width + font.letter_spacing;
        }
        string++;
    }
}
