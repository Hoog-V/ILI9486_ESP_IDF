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
#include "Fonts/Arial_32.h"
#include "Fonts/Cantarell Regular_32.h"
//#include "Arial_64.h"
//#include "Arial_8.h"
#include "Fonts/Arial_24.h"
#include "Fonts/DejaVu Serif_32.h"

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
    fillRect(spi, 0, 0, 320, 480, BLACK);
    const char test[] = "abcdefghijklmnop";
    drawText(spi, 22, 100, test, color565(0,255,0), Cantarell_32);
    printf("This the orig string: %s \r\n",test);
    while(1){
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
