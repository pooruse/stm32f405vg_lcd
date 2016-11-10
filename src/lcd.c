#include "spi.h"
#include "lcd.h"
#include "gpio.h"

/* basic instructions */
#define CLEAR 0x01
#define HOME 0x02

#define ENTRY 0x04
#define ENTRY_ID 0x01
#define ENTRY_S 0x02

#define DISPLAY 0x08
#define DISPLAY_D 0x04
#define DISPLAY_C 0x02
#define DISPLAY_B 0x01

#define CURSOR 0x10
#define CURSOR_SC 0x08
#define CURSOR_RL 0x04

#define FUNCTION 0x20
#define FUNCTION_DL 0x10
#define FUNCTION_RE 0x04

#define CGRAM 0x40
#define CGRAM_ADDR_MASK 0x3F

#define DDRAM 0x80
#define DDRAM_ADDR_MASK 0x7F

#define READ_BUSY_ADDR 0x200
#define READ_BUSY_MASK 0x80
#define READ_ADDR 0x3F

#define WRITE 0x100
#define WRITE_MASK 0xFF

#define READ_DATA 0x300
#define READ_DATA_MASK 0xFF


/* extended instructions */
#define STAND_BY 0x01

#define VERTICAL_SCROLL_OR_RAM_ADDR_SELECT 0x02
#define VERTICAL_SCROLL_OR_RAM_ADDR_SELECT_SR 0x01

#define REVERSE 0x04
#define REVERSE_R0 0x01
#define REVERSE_R1 0x02

#define SLEEP 0x08
#define SLEEP_SL 0x04

#define EXTENDED 0x20
#define EXTENDED_DL 0x10
#define EXTENDED_RE 0x04
#define EXTENDED_G 0x02

#define IRAM_OR_SCROLL 0x40
#define IRAM_OR_SCROLL_MASK 0x3F

#define GRAPHIC_RAM_ADDR 0x80
#define GRAPHIC_RAM_ADDR_MASK 0x7F

static uint8_t lcd_read(uint8_t command);

static void one_byte_write(uint32_t data);
static void two_byte_write(uint32_t data, uint8_t byte2);

static void wait_busy(void);
static void send_sync(uint8_t cmd);
static void send_byte(uint8_t data);

int delay;

void lcd_init(void){

    int sel;
    uint32_t dat;
    uint8_t u8dat;
    spi_init();

    /*
    one_byte_write(FUNCTION | FUNCTION_DL);
    wait_busy();
    one_byte_write(FUNCTION | FUNCTION_DL);
    wait_busy();

    one_byte_write(DISPLAY | DISPLAY_D | DISPLAY_C | DISPLAY_B);
    wait_busy();

    one_byte_write(CLEAR);
    for(delay=0;delay<200000;delay++);

    one_byte_write(ENTRY | ENTRY_S);
    wait_busy();

    one_byte_write(HOME);
    wait_busy();

    one_byte_write(CURSOR | CURSOR_RL);
    one_byte_write(DDRAM);
    
    */
    sel = 0;
    dat = 0;
    u8dat = 0;
    while(1){
	gpio_toggle_debug_led();
	switch(sel){
	case 0:
	    one_byte_write(dat);
	    break;
	case 1:
	    two_byte_write(dat, u8dat);
	    break;
	default:
	    break;
	}
    }
}

/** @brief LCD instruction bit composition
 *     uint8_t instruction[3];
 *     instruction[2] = {5'b1,RS,RW,1'b0}
 *     instruction[1] = {data[7:4], 4'b0}
 *     instruction[0] = {data[3:0], 4'b0}
 *  please refer the LCD datasheet VG12864Z-KBW-S15.pdf
 *  page 26 Timming Diagram of Serial Mode Data Transfer
 */

static uint8_t lcd_read(uint8_t command){
    
    uint8_t cmd,high_byte,low_byte;

    cmd = command;
    cmd <<= 1;
    cmd |= 0xF8;
    
    spi_tx(cmd);
    high_byte = spi_rx();
    low_byte = spi_rx();
    return (high_byte & 0xF0) | ( (0xF0 & low_byte) >> 4);
}

static void one_byte_write(uint32_t data){
    
    uint8_t cmd;
    uint8_t payload;

    
    cmd = (uint8_t)(data >> 8);
    payload = (uint8_t)data;

    spi_cs(1);
    send_sync(cmd);
    send_byte(payload);
    spi_cs(0);
}

static void two_byte_write(uint32_t data, uint8_t byte2){

    uint8_t cmd;
    uint8_t payload;

    cmd = (uint8_t)(data >> 8);
    payload = (uint8_t)data;
    
    spi_cs(1);
    send_sync(cmd);
    send_byte(payload);
    send_byte(byte2);
    spi_cs(0);
}

static void wait_busy(){

#ifdef CHECK_BUSY
    uint8_t tmp;

    do {
	tmp = lcd_read(READ_BUSY_ADDR);
	tmp &= READ_BUSY_MASK;
    }while(tmp);
    
#else
    
    for(delay = 0; delay < 1500; delay++);
#endif
    
}

static void send_sync(uint8_t cmd){
    
    uint8_t tmp;
    
    tmp = cmd << 1;
    tmp |= 0xF8;
    spi_tx(tmp);
}

static void send_byte(uint8_t data){
    
    uint8_t high_byte, low_byte;
    
    high_byte = (uint8_t)(data & 0xF0);
    low_byte = (uint8_t)((data & 0x0F) << 4);
    
    spi_tx(high_byte);
    spi_tx(low_byte);
}
