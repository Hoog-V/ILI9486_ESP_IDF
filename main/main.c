#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_task_wdt.h>
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lib/lcd_ili9486_base.h"
#include "lib/lcd_ili9486_primitives.h"
#include "lib/lcd_ili9486_colors.h"
#include "lib/lcd_ili9486_fontparser.h"
#include "lib/lcdfont.h"
#include "Fonts/basic_8.h"
//#include "Arial_32.h"
//#include "Arial_64.h"
//#include "Arial_8.h"
#include "Fonts/Arial_24.h"



struct lcdfont Arial_24 = {Arial24_char_addr, 
                           Arial24_char_width, 
                           Arial24_height, 
                           Arial24_Ybits,
                           Arial24_LSNorm,
                           VARIABLE_WIDTH,
                           NORMAL};

struct lcdfont Basic_8 = {Basic8_char_addr,
                          &Basic8_width,
                          Basic8_height,
                          Basic8_Ybits,
                          Basic8_LSNorm,
                          CONSTANT_WIDTH,
                          FLIPPED};

spi_device_handle_t setup_spi(){
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
    //Initialize the SPI bus
    ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(LCD_HOST, &ILI9486_devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    return spi;
}

void app_main(void)
{
    spi_device_handle_t spi = setup_spi();
    lcd_init(spi);
    fillRect(spi, 0, 0, 320, 480, 0x0000);
    const char test[] = "Test 123";
    drawText(spi, 22, 100, test, Basic_8);
    //drawChar(spi, 22, 100, numtoascii(0), Basic_8);
    printf("This the orig string: %s \r\n",test);
    while(1){
    vTaskDelay(1000/portTICK_PERIOD_MS);
    // for(int i =0; i< 10; i++){
    //     drawChar(spi, 22, 100, numtoascii(i), Arial_24);
    //     vTaskDelay(1000/portTICK_PERIOD_MS);
    //     fillRect(spi, 22, 100, 24, 24, 0x0000);
    //
    }
}
