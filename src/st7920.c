#include "spi.h"
#include "st7920.h"
#include "font.h"

static void lcd_draw_font(int f, int x, int y);
static void lcd_draw_8x8(uint8_t *but, int x, int y);
static void lcd_draw_clear(void);

static void set_addr(int x, int y);
static uint8_t read(uint8_t command);
static void write(uint32_t data);
static void wait_busy(void);
static void send_sync(uint8_t cmd);
static void send_byte(uint8_t data);

int delay;

void lcd_init(void){
    
    spi_init();
    
    // make lcd run in the 8'bit mode
    write(FUNCTION | FUNCTION_DL);
    
    // normal command mode
    write(FUNCTION | FUNCTION_DL);

    // open display
    write(DISPLAY | DISPLAY_D);

    write(ENTRY);

    write(HOME);
    
    // clear ddram (note: can't clear graphic ram)
    write(CLEAR);
    for(delay=0;delay<2000000;delay++);

    lcd_draw_clear();

    lcd_draw_font(0, 0, 0);
    lcd_draw_font(1, 1, 0);
    lcd_draw_font(2, 2, 0);
    lcd_draw_font(3, 3, 0);
    lcd_draw_font(4, 4, 0);
    lcd_draw_font(5, 5, 0);
    lcd_draw_font(6, 6, 0);
    
    /*
    {
	uint8_t buf[8] = {
	    0xFF,0xFF,0xFF,0xFF,
	    0xFF,0xFF,0xFF,0xFF,
	};
	lcd_draw_8x8(buf,0,2);
    }
    */
    /*
    set_addr(0,0);
    write(WRITE | 0xFF);
    write(WRITE | 0xFF);
    */
    
}

static void lcd_draw_font(int f, int x, int y)
{
    lcd_draw_8x8(&font[f*8],x,y);
}

static void lcd_draw_8x8(uint8_t *buf, int x, int y){
    int i;
    for(i = 0; i < 8; i++){
	set_addr(x,y+i);
	write(WRITE | buf[i]);
    }
}

static void lcd_draw_clear(void){
    int i,j;
    for(i = 0; i < 32; i++){
	set_addr(0, i);
	for(j = 0; j < 32; j++){
	    write(WRITE | 0x0);
	}
    }
}

static void set_addr(int x, int y){
    write(FUNCTION | FUNCTION_G | FUNCTION_RE | FUNCTION_DL);
    write(GRAPHIC_RAM_ADDR | y);
    write(GRAPHIC_RAM_ADDR | x);
    write(FUNCTION | FUNCTION_DL);
}

/** @brief LCD instruction bit composition
 *     uint8_t instruction[3];
 *     instruction[2] = {5'b1,RS,RW,1'b0}
 *     instruction[1] = {data[7:4], 4'b0}
 *     instruction[0] = {data[3:0], 4'b0}
 *  please refer the LCD datasheet VG12864Z-KBW-S15.pdf
 *  page 26 Timming Diagram of Serial Mode Data Transfer
 */
static void write(uint32_t data){
    
    uint8_t cmd;
    uint8_t payload;

    cmd = (uint8_t)(data >> 8);
    payload = (uint8_t)data;

    spi_cs(1);
    send_sync(cmd);
    send_byte(payload);
    spi_cs(0);
    wait_busy();
}

static void wait_busy(){

#ifdef CHECK_BUSY
    uint8_t tmp;

    do {
	tmp = lcd_read(READ_BUSY_ADDR);
	tmp &= READ_BUSY_MASK;
    }while(tmp);
    
#else
    
    for(delay = 0; delay < 3000; delay++);
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
