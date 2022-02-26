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

#define CMD_SLPOUT 0x11
#define CMD_PWCTRL1 0xC0
#define CMD_PWCTRL2 0xC1
#define CMD_PWCTRL3 0xC2
#define CMD_VMCTRL  0xC5
#define CMD_FRMCTR1 0xB1
#define CMD_INVTR 0xB4
#define CMD_DISCTRL 0xB6
#define CMD_PGAMCTRL 0xE0
#define CMD_NGAMCTRL 0xE1
#define CMD_COLMOD 0x3A
#define CMD_RAMRDRC 0x3E

#define CMD_INVOFF  0x20 // Display Inversion OFF
#define CMD_INVON   0x21 // Display Inversion ON
#define CMD_CASET   0x2A // Display On
#define CMD_PASET   0x2B // Page Address Set
#define CMD_RAMWR   0x2C // Memory Write
#define CMD_MADCTL  0x36 // Memory Data Access Control
// BIT 7-2 Parameter of MADCTL
#define MADCTL_MY   0x80 
#define MADCTL_MX   0x40
#define MADCTL_MV   0x20
#define MADCTL_ML   0x10
#define MADCTL_BGR  0x08 
#define MADCTL_MH   0x04 
#define CMD_WDBVAL  0x51
#define CMD_CDBVAL  0x53

typedef struct {
    uint8_t cmd;
    uint16_t data[20];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

void send_lines(spi_device_handle_t spi, uint16_t ypos, uint16_t *linedata, uint16_t numlines);
void send_line_finish(spi_device_handle_t spi);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
void lcd_data(spi_device_handle_t spi, const uint16_t *data, int len);
void  lcd_cmd16(spi_device_handle_t spi, const uint8_t cmd);
void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
uint32_t lcd_get_id(spi_device_handle_t spi);
void lcd_init(spi_device_handle_t spi);
bool setCursor(spi_device_handle_t spi, uint16_t x, uint16_t y);
