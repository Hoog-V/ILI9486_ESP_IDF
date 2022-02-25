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
    /* Power Control 1 */
    {CMD_PWCTRL1, {0x1900, 0x1A00}, 4},
    /* Power Control 2 */
    {CMD_PWCTRL2, {0x4500, 0x0000}, 4},
    /* Power Control 3 */
    {CMD_PWCTRL3, {0x3300},2},
    /* VCOM Control */
    {CMD_VMCTRL, {0x0000, 0x2800},4},
    /* Frame Rate Control */
    {CMD_FRMCTR1, {0xA000, 0x1100},4},
    /* Display Inversion Control 2-dot inversion */
    {CMD_INVTR, {0x0200}, 2},
    /* Display Function Control */
    {CMD_DISCTRL, {0x0000, 0x4200, 0x3B00},6},
    /* Positive gamma correction */
    {CMD_PGAMCTRL, {0x1F00, 0x2500, 0x2200, 0x0B00, 0x0600, 0x0A00, 0x4E00, 0xC600, 0x3900, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, 30},
    /* Negative gamma correction */
    {CMD_NGAMCTRL, {0x1F00, 0x3F00, 0x3F00, 0x0F00, 0x1F00, 0x0F00, 0x4600, 0x4900, 0x3100, 0x0500, 0x0900, 0x0300, 0x1C00, 0x1A00, 0x0000}, 30},
    /* Interface Pixel Format */
    {CMD_COLMOD, {0x5500}, 2},
    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    {CMD_DISCTRL, {0x0000, 0x2200},4},
    /* Memory Access Control */
    {CMD_MADCTL, {0x0800}, 2},
    /* Display Sleepout */
    {CMD_SLPOUT, {0}, 0x80},
    /* Done :) */
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
    lcd_cmd(spi, 0x04);

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


/* To send a set of lines we have to send a command, 2 data bytes, another command, 2 more data bytes and another command
 * before sending the line data itself; a total of 6 transactions. (We can't put all of this in just one transaction
 * because the D/C line needs to be toggled in the middle.)
 * This routine queues these commands up as interrupt transactions so they get
 * sent faster (compared to calling spi_device_transmit several times), and at
 * the mean while the lines for next transactions can get calculated.
 */
void send_lines(spi_device_handle_t spi, uint16_t ypos, uint16_t *linedata, uint16_t numlines)
{
    esp_err_t ret;
    int x;
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans[8];

    // // //In theory, it's better to initialize trans and data only once and hang on to the initialized
    // // //variables. We allocate them on the stack, so we need to re-init them each call.
     for (x=0; x<8; x++) {
         memset(&trans[x], 0, sizeof(spi_transaction_t));
     }
    trans[0].tx_data[0]=0x00;
    trans[0].tx_data[1]=0x2A;
    trans[0].length = 16;
    trans[0].user =(void*)0;
    trans[0].flags = SPI_TRANS_USE_TXDATA;
    trans[1].tx_data[0]=0x00;
    trans[1].tx_data[1]=0x00;// XSTART
    trans[1].tx_data[2]=0x00;
    trans[1].tx_data[3]=0x00;
    trans[1].length = 32;
    trans[1].user =(void*)1;
    trans[1].flags = SPI_TRANS_USE_TXDATA;
    trans[2].tx_data[0]=0x00;
    trans[2].tx_data[1]=0x01; //XEND
    trans[2].tx_data[2]=0x00;
    trans[2].tx_data[3]=0x3F;
    trans[2].length = 32;
    trans[2].user =(void*)1;
    trans[2].flags = SPI_TRANS_USE_TXDATA;
    trans[3].tx_data[0]=0x00;
    trans[3].tx_data[1]=0x2A;  //PASET
    trans[3].length = 16;
    trans[3].user =(void*)0;
    trans[3].flags = SPI_TRANS_USE_TXDATA;
    trans[4].tx_data[0]=0x00; //YSTART
    trans[4].tx_data[1]=ypos >> 8;
    trans[4].tx_data[2]=0x00;
    trans[4].tx_data[3]=ypos & 0xFF;
    trans[4].length = 32;
    trans[4].user =(void*)1;
    trans[4].flags = SPI_TRANS_USE_TXDATA;
    trans[5].tx_data[0]=0x00;
    trans[5].tx_data[1]=480 >> 8;  //YEND
    trans[5].tx_data[2]=0x00;
    trans[5].tx_data[3]=480 & 0xFF;
    trans[5].length = 32;
    trans[5].user =(void*)1;
    trans[5].flags = SPI_TRANS_USE_TXDATA;
    trans[6].tx_data[0]=0x00;
    trans[6].tx_data[1]=0x2C;
    trans[6].length = 16;
    trans[6].user =(void*)0;
    trans[6].flags = SPI_TRANS_USE_TXDATA;
    trans[7].tx_buffer=linedata;        //finally send the line data
    trans[7].length=320*2*8;          //Data length, in bits
    trans[7].flags=0; //undo SPI_TRANS_USE_TXDATA flag
    trans[7].user =(void*)1;

    //Queue all transactions.
    for (x=0; x<8; x++) {
        ret=spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }
    for(int i = 0; i < numlines; i++){
        ret=spi_device_queue_trans(spi, &trans[7], portMAX_DELAY);
        assert(ret==ESP_OK);
    }
    //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
    //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
    //finish because we may as well spend the time calculating the next line. When that is done, we can call
    //send_line_finish, which will wait for the transfers to be done and check their status.
}


void send_line_finish(spi_device_handle_t spi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all 6 transactions to be done and get back the results.
    for (int x=0; x<8; x++) {
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
}



