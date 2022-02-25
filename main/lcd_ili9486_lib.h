#pragma once

#define LCD_HOST    HSPI_HOST
#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15

#define PIN_NUM_TP_CS   26
#define PIN_NUM_DC   33
#define PIN_NUM_RST  27
#define PIN_NUM_BCKL 5

#define LCD_Heigth 480
#define LCD_Width 320


typedef struct {
    uint8_t cmd;
    uint16_t data[20];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

void send_lines(spi_device_handle_t spi, uint16_t ypos, uint16_t *linedata, uint16_t numlines);
void send_line_finish(spi_device_handle_t spi);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
void  lcd_cmd16(spi_device_handle_t spi, const uint8_t cmd);
void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
uint32_t lcd_get_id(spi_device_handle_t spi);
void lcd_init(spi_device_handle_t spi);
