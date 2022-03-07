#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_ili9486_lib.h"
DRAM_ATTR static const lcd_init_cmd_t ili_init_cmds[]={
    /* Display Sleepout */
    {CMD_SLPOUT, {0}, 0x80},
    /* Power Control 3 */
    {CMD_PWCTRL3, {0x4400},2},
    /* VCOM Control */
    {CMD_VMCTRL, {0x0000, 0x0000, 0x0000, 0x0000},8},
    /* Positive gamma correction */
    {CMD_PGAMCTRL, {0x1F00, 0x2500, 0x2200, 0x0B00, 0x0600, 0x0A00, 0x4E00, 0xC600, 0x3900, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, 30},
    /* Negative gamma correction */
    {CMD_NGAMCTRL, {0x1F00, 0x3F00, 0x3F00, 0x0F00, 0x1F00, 0x0F00, 0x4600, 0x4900, 0x3100, 0x0500, 0x0900, 0x0300, 0x1C00, 0x1A00, 0x0000}, 30},
    /* Interface Pixel Format */
    {CMD_COLMOD, {0x5500}, 2},
    /* Memory Access Control */
    {CMD_MADCTL, {0x4800}, 2},
    /* Turn off display inversion*/
    {CMD_INVOFF, {0}, 0x80},    //ets_delay_us(1);
    /* Display Sleepout */
    {CMD_SLPOUT, {0}, 0x80},
    {CMD_DISON, {0},0x80},
    /* END of cmdlist :) */
    {0, {0}, 0xff}
};

uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
    uint16_t conv = ((color & 0x00FF) << 8) | ((color & 0xFF00) >> 8);
    return conv;
}

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}


/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, const uint16_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}



void  lcd_cmd16(spi_device_handle_t spi, const uint8_t cmd){
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=16;                     //Command is 8 bits
    t.tx_data[0]=0x00;
    t.tx_data[1]=cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    t.flags = SPI_TRANS_USE_TXDATA;
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

uint32_t lcd_get_id(spi_device_handle_t spi)
{
    //get_id cmd
    lcd_cmd16(spi, 0x04);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8*3;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;

    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*)t.rx_data;
}

//Initialize the display
void lcd_init(spi_device_handle_t spi)
{
    int cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;

    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_TP_CS, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(PIN_NUM_MISO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_NUM_MOSI, GPIO_PULLUP_ONLY);
    //Reset the display
    gpio_set_level(PIN_NUM_TP_CS, 1);
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(1000 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(1000 / portTICK_RATE_MS);

    
    //detect LCD type
    //uint32_t lcd_id = lcd_get_id(spi);
    //printf("LCD ID: %08X\n", lcd_id);
    //zero, ili
    //printf("ILI9341 detected.\n");
    //printf("LCD ILI9341 initialization.\n");
    lcd_init_cmds = ili_init_cmds;

    //Send all the commands
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd16(spi, lcd_init_cmds[cmd].cmd);
        lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        if (lcd_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 0);
}

void setWriteArea(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, uint16_t width, uint16_t height){
    esp_err_t ret;
    int x;
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans[7];

    // // //In theory, it's better to initialize trans and data only once and hang on to the initialized
    // // //variables. We allocate them on the stack, so we need to re-init them each call.
     for (x=0; x<7; x++) {
         memset(&trans[x], 0, sizeof(spi_transaction_t));
         trans[x].flags = SPI_TRANS_USE_TXDATA;
     }
    trans[0].tx_data[0]=0x00;
    trans[0].tx_data[1]=CMD_CASET;
    trans[0].length = 16;
    trans[0].user =(void*)0;
    
    trans[1].tx_data[0]=0x00;
    trans[1].tx_data[1]=xbegin >> 8;// XSTART
    trans[1].tx_data[2]=0x00;
    trans[1].tx_data[3]=xbegin & 0xFF;
    trans[1].length = 32;
    trans[1].user =(void*)1;

    trans[2].tx_data[0]=0x00;
    trans[2].tx_data[1]=(xbegin + width-1) >> 8; //XEND
    trans[2].tx_data[2]=0x00;
    trans[2].tx_data[3]=(xbegin + width-1) & 0xFF;
    trans[2].length = 32;
    trans[2].user =(void*)1;

    trans[3].tx_data[0]=0x00;
    trans[3].tx_data[1]=CMD_PASET;  //PASET
    trans[3].length = 16;
    trans[3].user =(void*)0;

    trans[4].tx_data[0]=0x00;
    trans[4].tx_data[1]=ybegin >> 8;// YSTART
    trans[4].tx_data[2]=0x00;
    trans[4].tx_data[3]=ybegin & 0xFF;
    trans[4].length = 32;
    trans[4].user =(void*)1;

    trans[5].tx_data[0]=0x00; //YSTOP
    trans[5].tx_data[1]=(ybegin+height-1) >> 8;
    trans[5].tx_data[2]=0x00;
    trans[5].tx_data[3]=(ybegin+height-1) & 0xFF;
    trans[5].length = 32;
    trans[5].user =(void*)1;

    trans[6].tx_data[0]=0x00;
    trans[6].tx_data[1]=CMD_RAMWR;
    trans[6].length = 16;
    trans[6].user =(void*)0;
    for (x=0; x<7; x++) {
      ret=spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
      assert(ret==ESP_OK);
    }
    spi_transaction_t *rtrans;
    for (int x=0; x<7; x++) {
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
}


void fillRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, uint16_t color)
{
    uint16_t* linebuffer = NULL;
    esp_err_t ret;
    static spi_transaction_t trans;
    heap_caps_free(linebuffer);
    linebuffer= heap_caps_malloc(2*width, MALLOC_CAP_DMA);
    setWriteArea(spi, xpos, ypos, width, height);
    for(int i =0; i<width; i++){
        linebuffer[i]=color;
    }
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.tx_buffer=linebuffer;       //finally send the line data
    trans.length=16*width;          //Data length, in bits
    trans.user =(void*)1;
    trans.flags=0; //undo SPI_TRANS_USE_TXDATA flag
    spi_transaction_t *rtrans;
    for(int i = 0; i < height; i++){
        ret=spi_device_queue_trans(spi, &trans, portMAX_DELAY);
        assert(ret==ESP_OK);
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        }
    heap_caps_free(linebuffer);
}

bool drawPixel(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t color) {

    if (x >= 320|| y >= 480) {
        return false;
    }
    esp_err_t ret;
    static spi_transaction_t trans[6];
    for (int i=0; i<6; i++) {
      memset(&trans[i], 0, sizeof(spi_transaction_t));
      trans[i].flags = SPI_TRANS_USE_TXDATA;
      if((i+1) % 2){
          trans[i].user = (void*)0;
      }
      else{
          trans[i].user = (void*)1;
      }
    }
    trans[0].tx_data[0]=0x00;
    trans[0].tx_data[1]=CMD_CASET;
    trans[0].length = 16;
    trans[1].tx_data[0]=0x00;
    trans[1].tx_data[1]=(x>>8);
    trans[1].tx_data[2]= 0x00;
    trans[1].tx_data[3]=(x&0xFF);
    trans[1].length = 32;
    trans[2].tx_data[0]= 0x00;
    trans[2].tx_data[1]= CMD_PASET;
    trans[2].length = 16;
    trans[3].tx_data[0] = 0x00;
    trans[3].tx_data[1] = (y>>8);
    trans[3].tx_data[2] = 0x00;
    trans[3].tx_data[3] = (y&0xFF);
    trans[3].length = 32;
    trans[4].tx_data[0] = 0x00;
    trans[4].tx_data[1] = CMD_RAMWR;
    trans[4].length = 16;
    trans[5].tx_data[0] = (uint16_t)(color & 0x00FF);
    trans[5].tx_data[1] = color >> 8;
    trans[5].length=16;
    trans[5].flags = SPI_TRANS_USE_TXDATA;
    trans[5].user = (void *)1;
    ret=spi_device_queue_trans(spi, &trans[0], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[1], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[1], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[2], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[3], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[3], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[4], portMAX_DELAY);
    ret |=spi_device_queue_trans(spi, &trans[5], portMAX_DELAY);
    assert(ret==ESP_OK);
    return true;
}

void drawVLine(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, uint16_t length, uint16_t color){
    uint16_t* linebuffer = NULL;
    setWriteArea(spi, xbegin, ybegin, 1, length);
    esp_err_t ret;
    static spi_transaction_t trans;
    linebuffer= heap_caps_malloc(2*length, MALLOC_CAP_DMA);
    for(int i =0; i<length; i++){
        linebuffer[i]=color;
    }
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.tx_buffer=linebuffer;       //finally send the line data
    trans.length=16*length;          //Data length, in bits
    trans.user =(void*)1;
    trans.flags=0; //undo SPI_TRANS_USE_TXDATA flag
    ret=spi_device_queue_trans(spi, &trans, portMAX_DELAY);
    assert(ret==ESP_OK);
    spi_transaction_t *rtrans;
    ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
    assert(ret==ESP_OK);
    //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    heap_caps_free(linebuffer);
}

void drawHLine(spi_device_handle_t spi, uint16_t xbegin, uint16_t ybegin, uint16_t length, uint16_t color){
    uint16_t* linebuffer = NULL;
    setWriteArea(spi, xbegin, ybegin, length, 1);
    esp_err_t ret;
    static spi_transaction_t trans;
    linebuffer= heap_caps_malloc(2*length, MALLOC_CAP_DMA);
    for(int i =0; i<length; i++){
        linebuffer[i]=color;
    }
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.tx_buffer=linebuffer;       //finally send the line data
    trans.length=16*length;          //Data length, in bits
    trans.user =(void*)1;
    trans.flags=0; //undo SPI_TRANS_USE_TXDATA flag
    ret=spi_device_queue_trans(spi, &trans, portMAX_DELAY);
    assert(ret==ESP_OK);
    spi_transaction_t *rtrans;
    ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
    assert(ret==ESP_OK);
    //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    
    heap_caps_free(linebuffer);
}

void drawRect(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, uint16_t color){
    drawVLine(spi, xpos, ypos, height, color);
    drawVLine(spi, (xpos+width), ypos, height, color);
    drawHLine(spi, xpos, ypos, width, color);
    drawHLine(spi, xpos, (ypos+height), width, color);
}