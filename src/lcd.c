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
#define FUNCTION_G 0x02

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

uint8_t font[][8] = {
    {0x00,0x40,0x60,0x70,0x78,0x7C,0X7E,0X7F},
    {0x00,0X7E,0X7C,0X78,0X70,0X60,0X40,0x00}, // UP
    {0x00,0x44,0x44,0x5F,0x44,0x44,0X00,0X00},    
    {0x00,0x10,0x38,0x7C,0xFE,0x00,0X00,0X00}, // +-	
    {0x00,0xFE,0x7C,0x38,0x10,0x00,0X00,0X00}, // R
    {0x00,0x04,0x0C,0x1C,0x3C,0x1C,0X0C,0X04}, // DOWN	
    {0x00,0x40,0x60,0x70,0x78,0x70,0X60,0X40}, // UP
    {0x00,0x00,0x3e,0x51,0x49,0x45,0x3e,0x00}, // 0  -0x30
    {0x00,0x00,0x40,0x42,0x7f,0x40,0x40,0x00}, // 1 
    {0x00,0x00,0x42,0x61,0x51,0x49,0x46,0x00}, // 2 
    {0x00,0x00,0x21,0x41,0x45,0x4b,0x31,0x00}, // 3 
    {0x00,0x00,0x18,0x14,0x12,0x7f,0x10,0x00}, // 4 
    {0x00,0x00,0x27,0x45,0x45,0x45,0x39,0x00}, // 5
    {0x00,0x00,0x3c,0x4a,0x49,0x49,0x30,0x00}, // 6
    {0x00,0x00,0x01,0x71,0x09,0x05,0x03,0x00}, // 7 
    {0x00,0x00,0x36,0x49,0x49,0x49,0x36,0x00}, // 8 
    {0x00,0x00,0x06,0x49,0x49,0x29,0x1e,0x00}, // 9
};

static void lcd_draw_font(int f, int x, int y);
static void lcd_draw_8x8(uint8_t *but, int x, int y);
static void lcd_draw_clear(void);
static void lcd_set_addr(int x, int y);
static uint8_t read(uint8_t command);
static void write(uint32_t data);
static void wait_busy(void);
static void send_sync(uint8_t cmd);
static void send_byte(uint8_t data);

int delay;

void lcd_init(void){
    
    //uint32_t dat = 0;
    spi_init();
    
    /*
    while(1){	
	gpio_toggle_debug_led();
	write(dat);
    }
    */
    
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
    lcd_set_addr(0,0);
    write(WRITE | 0xFF);
    write(WRITE | 0xFF);
    */
    
}

static void lcd_draw_font(int f, int x, int y)
{
    lcd_draw_8x8(font[f],x,y);
}

static void lcd_draw_8x8(uint8_t *buf, int x, int y){
    int i;
    for(i = 0; i < 8; i++){
	lcd_set_addr(x,y+i);
	write(WRITE | buf[i]);
    }
}

static void lcd_draw_clear(void){
    int i,j;
    for(i = 0; i < 32; i++){
	lcd_set_addr(0, i);
	for(j = 0; j < 32; j++){
	    write(WRITE | 0x0);
	}
    }
}

static void lcd_set_addr(int x, int y){
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
