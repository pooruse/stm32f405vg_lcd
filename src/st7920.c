#include <string.h>
#include "spi.h"
#include "st7920.h"
#include "font.h"

#define SHIFT_TEST

static void copy_array_with_shift(
    uint8_t *src,
    uint8_t *dst,
    int size,
    int head_shift,
    int tail_shift);

static void lcd_draw_8x8(uint8_t *but, int x, int y);
static void lcd_draw_clear(void);

static void set_addr(int x, int y);
//static uint8_t read(uint8_t command);
static void write(uint32_t data);
static void wait_busy(void);
static void send_sync(uint8_t cmd);
static void send_byte(uint8_t data);

static uint8_t screen_buf[1024]; // 128 x 64 bit = 1024 byte
int delay;

void st7920_init(void){

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

    #ifdef SHIFT_TEST
    {
	int head = 3;
	int tail = 3;
	uint8_t src[5] = {
	    0x10, 0x18, 0xF8, 0xFF, 0xFF
	};
	uint8_t dst[6] = {
	    0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F
	};

	uint8_t expect[6] = {
	    0xE2, 0x03, 0x1F, 0x1F, 0xFF, 0xEF
	};

	int i;
	copy_array_with_shift(
	    src,
	    dst,
	    sizeof(src),
	    head,
	    tail
	    );
	
	for(i = 0; i < 6; i++){
	    if (dst[i] != expect[i] ){
		// assert
		while(1);
	    }
	}
    }
    #endif
    
}

/**
 *  @brief _draw_rectangle
 *     draw_rectangle driver (support halfword data)
 *
 *  @param xh
 *     lcd xh, range 0~7 (halfword)
 *     ex. 0, 1, 2, 3, 4, 5, 6, 7 
 *
 *  @param y
 *     lcd y, range 0~63 (bit)
 *
 *  @param wh
 *     screen width (halfword)
 *
 *  @param h
 *     screen height (bit)
 *  @caution
 *     if w or h is over the screen size, 
 *   this function have no protection for it.
 */
static void _draw_rectangle(int xh, int y, int wh, int h)
{
    int tmpy;
    int tmpxh;
    int i ,j;

    for(j = y; j < (y + h); j++){
	for(i = xh; i < (xh + wh); i++){
	    
	    // if y > 31
	    if(y & 0x20){
		tmpxh = i | 0x8;
		tmpy = j & 0x1F;
	    } else {
		tmpxh = i;
		tmpy = j;
	    }
	    
	    set_addr(tmpxh, tmpy);
	    write(WRITE | screen_buf[j*16 + i * 2]);
	    write(WRITE | screen_buf[j*16 + i * 2 + 1]);
	}
    }
}

/**
 *  @brief draw_rectangle
 *     lcd original point is on left top
 *     (x,y)
 *     (0,0) (1,0) (2,0) (3,0) ... (15,0)
 *     (0,1)
 *     (0,2)
 *       .
 *       .
 *       .
 *     (0,63)
 *
 *  @param xb
 *     lcd xb, range 0~7 (byte)
 *     ex. 0, 1, 2, 3, 4, 5, 6, 7 
 *
 *  @param y
 *     lcd y, range 0~63 (bit)
 *
 *  @param wb
 *     screen width (byte)
 *
 *  @param h
 *     screen height (bit)
 *  @caution
 *     if w or h is over the screen size, 
 *   this function have no protection for it.
 */
static void draw_rectangle(int xb, int y, int wb, int h)
{
    int xh,wh;
    int tmpxb, tmpwb;
    
    tmpxb = xb;
    tmpwb = wb;
    
    if(tmpxb % 2){
	xb -= 1;
	wb++;
    }

    
    if((tmpwb + tmpxb) % 2){
	wb++;
    }

    // spection case
    if(wb == 1){
	wb++;
    }

    xh = xb/2;
    wh = wb/2;
    _draw_rectangle(xh, y, wh, h);
}

static void draw_parameter_modify(struct st7920_draw_rectangle_t *draw){
    // x exception
    if( draw->x > 127 ){
	draw->x = 127;
    } else {
	draw->x = draw->x;
    }

    // y exception
    if( draw->y > 63 ){
	draw->y = 63;
    } else {
	draw->y = draw->y;
    }

    // w exception
    if( (draw->x + draw->w) > 128 ){
	draw->w = 128 - draw->x;
    } else {
	draw->w = draw->w;
    }

    // h exception
    if( (draw->y + draw->h) > 64 ){
	draw->h = 64 - draw->y;
    } else {
	draw->h = draw->h;
    }
}

static void copy_array_with_shift(
    uint8_t *src,
    uint8_t *dst,
    int size,
    int head_shift,
    int tail_shift){
    
    uint8_t msb, lsb;
    uint8_t mask;
    int i;

    if(size >= 16){
	size = 15;
    }

    // when head = 3, mask = 0001 1111
    mask = (1 << (8 - head_shift)) - 1;

    // start byte
    dst[0] &= ~mask;
    dst[0] |= (src[0] >> head_shift);

    
    // when tail = 5, mask = 0000 0111
    mask = (1 << (8 - tail_shift)) - 1;

    // end byte
    dst[size] = dst[size];
    dst[size] &= mask;
    dst[size] |= (src[size - 1] << (8 - tail_shift));

    // middle bytes
    for(i = 1; i < size; i++){	
	msb = src[i - 1] << (8 - head_shift);
	lsb = src[i] >> head_shift;
	dst[i] = msb | lsb;
    }
}    

/** @brief how to draw the screen per line
 *     
 *         one line in LCD Screen
 *       
 *         +--------------------------+----   --+
 *     bit |76543210|76543210|76543210|7654...10|
 *         +--------+--------+--------+----   --+
 *    byte 0        1        2        3    ...  15
 *
 *    case 1: complete_data
 *            x = 8, w = 8
 *                  +--------+
 *                  |76543210|
 *                  +--------+
 *            result: screen_buf_byte1 = new_data_byte1
 *
 *    case 2: start_byte_part + complete_data
 *            x = 5, w = 13 
 *   
 *               ---+--------+
 *               210|76543210|
 *               ---+--------+
 *            result: 1. screen_buf_byte0 = {screen_buf_byte0[7:3], new_data_byte0[2:0]}
 *                    2. screen_buf_byte1 = new_data_byte1
 *
 *    case 3: complete_data + end_byte_part
 *            x = 8, w = 10
 *                  +--------+--
 *                  |76543210|76
 *                  +--------+--
 *            result: 1. screen_buf_byte1 = new_data_byte1
 *                    2. screen_buf_byte2 = {new_data_byte2[7:6] ,screen_buf_byte2[5:0]}
 *
 *    case 4: start_byte_part + complete_data + end_byte_part
 *            x = 2, w = 20 
 *            ------+--------+------
 *            543210|76543210|765432
 *            ------+--------+------
 *            result: 1. screen_buf_byte0 = {screen_buf_byte0[7:6], new_data_byte0[5:0]}
 *                    2. screen_buf_byte1 = new_data_byte1
 *                    3. screen_buf_byte2 = {new_data_byte2[7:2] ,screen_buf_byte2[1:0]}
 *                    
 *
 */
void lcd_draw_rectangle(struct st7920_draw_rectangle_t draw)
{
    int x,y,w,h; // unit: 1 point
    
    // xb = x index (unit: 8 point) 
    int xb,wb;
    
    int head_shift, end_part_len;

    uint8_t *buf;
    int j;

    draw_parameter_modify(&draw);
    
    x = draw.x;
    y = draw.y;
    w = draw.w;
    h = draw.h;
    buf = draw.buf;
    xb = x/8;
    wb = w/8;

    if( (wb * h) > draw.size){
	return;
    }

    head_shift = x % 8;
    end_part_len = (x + w) % 8;
    
    if(head_shift != 0 ||
       end_part_len != 0) {
	wb = wb + 1;
    }

    // copy graphic to screen_buf
    for(j = y; j < (h + y); j++){
	copy_array_with_shift(
	    &buf[(wb - 1) * j],
	    &screen_buf[16 * j + xb],
	    (wb - 1),
	    head_shift,
	    end_part_len);
    }
    
    draw_rectangle(xb, y, wb, h);
}

void lcd_draw_font(int f, int x, int y)
{
    lcd_draw_8x8((uint8_t *)&font[f*8],x,y);
}

static void lcd_draw_8x8(uint8_t *buf, int x, int y){

    struct st7920_draw_rectangle_t draw;
    
    draw.x = x;
    draw.y = y;
    draw.w = 8;
    draw.h = 8;
    draw.buf = buf;
    draw.size = 8;
    
    lcd_draw_rectangle(draw);
}

static void lcd_draw_clear(void){
    int i,j;
    for(i = 0; i < 32; i++){
	set_addr(0, i);
	for(j = 0; j < 32; j++){
	    write(WRITE | 0x00);
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
