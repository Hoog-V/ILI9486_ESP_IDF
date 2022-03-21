#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_ili9486_base.h"
#include "lcdfont.h"



void drawChar(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, char character, struct lcdfont font) {
    int x,y;
    int set;
    const char *bitmap = font.char_addr[character-48];
    uint8_t character_width = font.width[character-48];
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

// void render(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, char *bitmap) {
//     int x,y;
//     int set;
//     for (x=0; x < 6; x++) {
//     for(int p=0; p< 3; p++){
//      for (y=0; y < 8; y++) {
//         set = bitmap[x+(p*6)] & 1 << y;
//         if(set){
//             drawPixel(spi, xstart+x, ystart+y+(p*8), 0xFFFF);
//         }
//         }
//     }
//     }
// }

// void render(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, char *bitmap) {
//     int x,y;
//     int set;
//     for (x=0; x < 4; x++) {
//      for (y=0; y < 8; y++) {
//         set = bitmap[x] & 1 << y;
//         if(set){
//             drawPixel(spi, xstart+x, ystart+y, 0xFFFF);
//         }
//         }
//     }
// }

// void render(spi_device_handle_t spi, uint16_t xstart, uint16_t ystart, char *bitmap) {
//     int x =0, y = 0;
//     int it = 0;
//     int offset = 0;
//     int set;
//     do{
//         if(!((it+1) % 4)){
            
//             offset+=2;
//         }
//         x++;
//         for(y=0; y < 8; y++){
//              set = bitmap[it] & 1 << y;
//              if(set){
//                 drawPixel(spi, xstart+x, ystart+y+offset, 0xFFFF);
//             }
//         }
//         it++;
//     }
//     while(it < 68);
//     printf("X is now: %d \r \n", x);

// }