#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "ILI9486/lcd_ili9486_base.h"
#include "ILI9486/lcd_ili9486_fontparser.h"
#include "ILI9486/lcdfont.h"
#include "math.h"

#define ArrStartaddr 0x20

inline unsigned char getarrOffset(unsigned char character){
    return character-ArrStartaddr;
}


void print_bitmap16(unsigned char *buffer, uint8_t w, uint8_t h)
{
uint8_t arrindex = 0;
for(uint8_t y = 0; y <h; y++)
{
 for(uint8_t p = 0; p< (w/8); p++)
 {
	
	for(uint8_t x = 0; x < 8; x++)
	{
		printf("%c ", (buffer[arrindex] & (0x01  << (x) )) ? 'X' : ' ');
	}
	arrindex++;
 }
	printf("Buffer %d : %d \r\n", y, (uint16_t) (buffer[y] | ((buffer[y+1] & 0xFF)<<8)));
} 
}

void drawChar(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, 
        unsigned char character,uint16_t color, struct lcdfont font, uint8_t scale) 
{
    int x,y;
    int set;
    uint8_t character_width;
    uint16_t buffersize;
    
    uint8_t a_of_bytes = (font.font_size/font.line_size)*scale;
    printf("Amount of bytes: %d\r\n",a_of_bytes);
    const unsigned char *bitmap = (const unsigned char *)font.addr[getarrOffset(character)];
    if(font.variable_width){
        character_width = font.width[getarrOffset(character)]*scale;
    }
    else{
        character_width = (*font.width)*scale;
        
    }
    printf("character width: %d \r\n",character_width);
    buffersize =font.line_size*character_width;
    char scaled_bitmap[buffersize];
    scale_bitmap(bitmap, scaled_bitmap, scale);
    bool flipped = font.font_flipped;

    uint8_t arrindex = 0;
    for (y=0; y < character_width; y++) {
        for(int p=0; p< a_of_bytes; p++){
             for (x=0; x < font.line_size; x++) {
                 set = scaled_bitmap[arrindex] & (1 << x);
                 if(set && flipped)
                    drawPixel(spi, xstart+x+(p*8), ystart+y, color);
                 else if (set && !flipped)
                     drawPixel(spi, xstart+y, ystart+x+(p*8), color);
              }
              arrindex++;
        }
    }
    //print_bitmap16((unsigned char *)scaled_bitmap,8*scale,8*scale);

}

void drawText(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, const char *string,
        uint16_t color, struct lcdfont font, uint8_t textsize)
{
    uint16_t x = xstart;
    while(*string){

        drawChar(spi, x, ystart+5, *string, color, font,textsize);
        if(font.variable_width){
            x += font.width[getarrOffset((unsigned char)*string)] + font.letter_spacing*textsize;
        }
        else{
            x+= *font.width*textsize + font.letter_spacing;
        }

        string++;
    }
}


void scale_bitmap(const unsigned char *origbuffer, char *newbuffer, uint8_t scale)
{
	uint16_t arrindex = 0;
	for(uint16_t ys =0; ys < 8*scale; ys++)
	{
		uint16_t y = round(ys/scale);
		for(uint8_t p = 0; p < scale; p++)
		{
			
			for(uint8_t x = 0; x < 8; x++)
			{
				uint16_t t = round(((x)+(p*8))/scale);
				if(origbuffer[y] & (0x01  << (t+1)))
			   		newbuffer[arrindex] |= 1 << (x);
				else 
				   newbuffer[arrindex] &= ~( 1<<(x) );
			}
			arrindex++;
		}
	
	}

}
