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
//#include "font8x8_basic.h"
//#include "Arial_32.h"
//#include "Arial_64.h"
//#include "Arial_8.h"
#include "Fonts/Arial_24.h"




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
    struct lcdfont Arial_24;
    Arial_24.char_addr = Arial24_char_addr;
    Arial_24.width = Arial24_char_width;
    Arial_24.font_size = 24;
    Arial_24.line_size= 8;
    spi_device_handle_t spi = setup_spi();
    lcd_init(spi);
    fillRect(spi, 0, 0, 320, 480, 0x0000);
    for(int i =0; i< 10; i++){
        drawChar(spi, 22, 100,i+48, Arial_24);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        fillRect(spi, 22, 100, 24, 24, 0x0000);
    }
}
