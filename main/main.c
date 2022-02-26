#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_ili9486_lib.h"

void app_main(void)
{
 //vTaskDelay(1000/ portTICK_RATE_MS);
    esp_err_t ret;
    spi_device_handle_t spi;
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=LCD_Width*480*2+8
    };
    spi_device_interface_config_t devcfg={
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz=26*1000*1000,           //Clock out at 26 MHz
#else
        .clock_speed_hz=10*1000*1000,           //Clock out at 10 MHz
#endif
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=100,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    lcd_init(spi);
    static uint16_t repeatedLine[320];
    lcd_cmd16(spi, 0x29);
    vTaskDelay(1000/ portTICK_RATE_MS);
    for(int i = 0; i< 320; i++){
        repeatedLine[i] = 0x0000;
    }
    send_lines(spi, 0, repeatedLine, 480);
    send_line_finish(spi);  
    vTaskDelay(2000/ portTICK_RATE_MS);
    static uint16_t pixels[10];
    for(int i = 0; i < 10; i++){
        pixels[i]=color565(255,255,255);
    } 
    setCursor(spi, 200, 320);
    lcd_data(spi,pixels,20);
  
    while(1){
     vTaskDelay(100/ portTICK_RATE_MS);
    }
}
