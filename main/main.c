#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_ili9486_lib.h"
#include <math.h>
void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t spi;
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=LCD_Width*16
    };
    spi_device_interface_config_t devcfg={
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz=26*1000*1000,           //Clock out at 26 MHz
#else
        .clock_speed_hz=32*1000*1000,           //Clock out at 10 MHz
#endif
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=60,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    lcd_init(spi);
    fillRect(spi, 0, 0, 320, 480, 0x0000);
    float y;
    float z;
    int linethickness = 1;
    drawVLine(spi, 20, 300, 100, 0xFFFF);
    drawHLine(spi, 20, 400, 100, 0xFFFF);
    int k = 0;
    while(1){
    fillRect(spi, 0, 10, 320, 200, 0x0000);
    for(int p =0; p < linethickness; p++){
    for(int i = 0; i < 500; i++){
    y= (i+k)*0.0174363323;
    z= (sin(y*6)*20)+(99+p);
    drawPixel(spi, i, z, 0xFFFF);
    }
    }
    vTaskDelay(1/1000);
    k+=10;
    //  fillRect(spi, 0, 0, 0x0000, 320, 480);  
    //  vTaskDelay(1000/ portTICK_RATE_MS);
    //  fillRect(spi, 0, 0, 0xFFFF, 320, 480);  
      //printf("Amount of free size: %d \n",  heap_caps_get_free_size(MALLOC_CAP_DMA));
    }
}
