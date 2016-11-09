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

#define READ_BUSY_ADDR 0x100
#define READ_BUSY_MASK 0x80
#define READ_ADDR 0x3F

#define WRITE 0x200
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

uint8_t lcd_read(uint8_t command);
void lcd_write(uint32_t data);
void lcd_write_data(uint32_t data);
void lcd_wait_busy(void);

int delay;

void lcd_init(void){

    uint8_t addr;
    uint8_t dat;
    spi_init();

    lcd_write(FUNCTION | FUNCTION_DL);
    lcd_wait_busy();
    lcd_write(FUNCTION | FUNCTION_DL);
    lcd_wait_busy();

    lcd_write(DISPLAY | DISPLAY_D);
    lcd_wait_busy();
    lcd_write(DISPLAY | DISPLAY_D | DISPLAY_C | DISPLAY_B);
    lcd_wait_busy();

    lcd_write(CLEAR);
    for(delay=0;delay<20000;delay++);

    lcd_write(ENTRY | ENTRY_ID);
    lcd_wait_busy();

    lcd_write(HOME);
    lcd_wait_busy();

    lcd_write(CURSOR | CURSOR_RL);
    lcd_write(DDRAM);
    dat = 1;
    addr = 0;

    while(1){

	for(delay = 0; delay < 1000000; delay++);
	gpio_toggle_debug_led();
	lcd_write(DDRAM | addr);
	lcd_wait_busy();
	
	lcd_write(WRITE | 0x00);
	lcd_wait_busy();
	
	lcd_write(WRITE | 0x50);
	lcd_wait_busy();
	addr++;
	dat++;
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

uint8_t lcd_read(uint8_t command){
    
    uint8_t cmd,high_byte,low_byte;

    cmd = command;
    cmd <<= 1;
    cmd |= 0xF8;
    
    spi_tx(cmd);
    high_byte = spi_rx();
    low_byte = spi_rx();
    return (high_byte & 0xFF) | (low_byte >> 4);
}

void lcd_write(uint32_t data){
    
    uint8_t cmd, high_byte, low_byte;
    
    cmd = (uint8_t)(data >> 8);
    high_byte = (uint8_t)(data & 0xF0);
    low_byte = (uint8_t)((data & 0x0F) << 4);

    cmd <<= 1;
    cmd |= 0xF8;

    spi_tx(cmd);
    spi_tx(high_byte);
    spi_tx(low_byte);
}

void lcd_wait_busy(){

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


