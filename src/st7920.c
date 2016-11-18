#include <string.h>
#include "spi.h"
#include "st7920.h"
#include "stdio.h"
#include "font.h"

//#define SHIFT_TEST
//#define CHECK_BUSY

#define SCREEN_BUF_SIZE 1024 // 128 x 64 bit = 1024 byte
static void copy_array_with_shift(
    uint8_t *src,
    uint8_t *dst,
    int size,
    int head_shift,
    int tail_shift);

static void set_addr(int x, int y);
static void lcd_draw_font(int f);

static uint8_t read(uint32_t command);
static void write(uint32_t data);
static void wait_busy(void);
static void send_sync(uint8_t cmd);
static void send_byte(uint8_t data);

static uint8_t screen_buf[SCREEN_BUF_SIZE]; 

static int delay;
static int font_x = 0, font_y = 0;

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
    for(delay=0;delay<2000000;delay++){
	asm("");
    }
    //wait_busy();

    #ifdef SHIFT_TEST
    {
	int head = 8;
	int tail = 8;
	uint8_t src[5] = {
	    0x10, 0x18, 0xF8, 0xFF, 0xFF
	};
	uint8_t dst[6] = {
	    0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F
	};

	uint8_t expect[6] = {
	    0xF0, 0x10, 0x18, 0xF8, 0xFF, 0xFF
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
	// if y > 31
	if(j & 0x20){
	    tmpxh = xh | 0x8;
	    tmpy = j & 0x1F;
	} else {
	    tmpxh = xh;
	    tmpy = j;
	}
	set_addr(tmpxh, tmpy);  
	for(i = xh; i < (xh + wh); i++){	    
	    write(WRITE | screen_buf[j * 16 + i * 2]);
	    write(WRITE | screen_buf[j * 16 + i * 2 + 1]);
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


/** @brief copy_array_with_shift
 *    only support 1 byte shift   
 *
 */
static void copy_array_with_shift(
    uint8_t *src,
    uint8_t *dst,
    int size,
    int head_shift,
    int tail_shift){
    
    uint8_t msb, lsb;
    uint8_t mask;
    uint8_t tmp;
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
    dst[size] &= mask;
    tmp = (src[size - 1] << (8 - head_shift));
    tmp &= ~mask;
    dst[size] |= tmp;

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
    int j,k;

    draw_parameter_modify(&draw);
    
    x = draw.x;
    y = draw.y;
    w = draw.w;
    h = draw.h;
    buf = draw.buf;
    xb = x/8;
    wb = w/8;

    if(wb == 0){
	// special case
	wb = 1;
    }
    
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
    k = 0;
    for(j = y; j < (h + y); j++){
	copy_array_with_shift(
	    &buf[k],
	    &screen_buf[16 * j + xb],
	    (wb - 1),
	    head_shift,
	    end_part_len);

	k += wb - 1;
    }
    
    draw_rectangle(xb, y, wb, h);
}

void lcd_set_font_addr(int x, int y){
    font_x = x;
    font_y = y;
}

static void lcd_draw_font(int f)
{
    struct st7920_draw_rectangle_t draw;
    
    draw.x = (font_x * 6) + 1;
    draw.y = font_y * 8;
    draw.w = 5;
    draw.h = 7;
    draw.buf = (uint8_t *)&font[f * 8];
    draw.size = 8;

    font_x++;
    if(font_x >= 21){
	font_x = 0;
	font_y++;
    }
    
    if(font_y >= 8){
	font_y = 0;
    }
    
    lcd_draw_rectangle(draw);
}

void lcd_putc(char c){

    int a = 0;
    
    // numbers
    if(c >= '0' && c <= '9'){
	a = (int)c;
	a -= (int)'0';
    }
    
    // a ~ z
    else if(c >= 'a' && c <= 'z'){
	a = (int)c;
	a -= (int)'a';
	a += 36;
    }

    // A ~ Z
    else if(c >= 'A' && c <= 'Z'){
	a = (int)c;
	a -= (int)'A';
	a += 10;
    }
    
    // return
    else if(c == '\r'){
	font_x = 0;
	return;
    }
    
    // new line
    else if(c == '\n'){
	font_y++;
	if(font_y >= 8){
	    font_y = 0;
	}
	return;
    }

    else if(c == '\b'){
	if(font_x != 0){
	    font_x--;
	}
	return;
    }
    // space
    else if(c == ' '){
	a = 62;
    }

    // ?
    else if(c == '?'){
	a = 63;
    }

    // :
    else if(c == ':'){
	a = 64;
    }

    // .
    else if(c == '.'){
	a = 65;
    }

    // %
    else if(c == '%'){
	a = 66;
    }

    // /
    else if(c == '/'){
	a = 67;
    }

    // -
    else if(c == '-'){
	a = 68;
    } else {
	return;
    }
    
    lcd_draw_font(a);
}

void lcd_clear(void){

    int i,j;
    for(i = 0; i < 32; i++){
	set_addr(0, i);
	for(j = 0; j < 32; j++){
	    write(WRITE | 0x00);
	}
    }

    for(i = 0; i < SCREEN_BUF_SIZE; i++){
	screen_buf[i] = 0;
    }


}

static void set_byte_array_bit(uint8_t *buf, int pos, int value)
{
    uint8_t tmp;
    int index;
    int bit;
    bit = pos % 8;
    index = pos / 8;
    tmp = 0x80 >> bit;
    if(value != 0){
	buf[index] |= tmp;
    } else {
	buf[index] &= ~tmp;
    }
}

#define BAR_START (3 * 8 + 1) * 16 
void lcd_bar_set(int value){
    static int percentage = 0;
    int i, j;
    int index;
    
    
    if(value != percentage){
	if(value > percentage){
	    for(i = percentage; i < value; i++){
		set_byte_array_bit(&screen_buf[BAR_START + 16] ,  i + 14, 1);
	    }
	} else if (value < percentage){
	    for(i = value; i < percentage; i++){
		set_byte_array_bit(&screen_buf[BAR_START + 16] , i + 14, 0);
	    }
	}

	for(i = 2; i < 13; i++){
	    for(j = 0; j < 16; j++){
		index = BAR_START + i * 16 + j;
		screen_buf[index] = screen_buf[BAR_START + 16 + j];
	    }
	}
	percentage = value;
	
	draw_rectangle(0, 3 * 8, 16, 16);
    }
}


void lcd_bar_create(void){
    int i,j;
    for(i = 0; i < 256; i++){
	screen_buf[BAR_START - 16 + i] = 0;
    }

    // top
    i = 0;
    screen_buf[BAR_START + i++] = 0x00;
    screen_buf[BAR_START + i++] = 0x07;
    for(j = 0; j < 12; j++){
	screen_buf[BAR_START + i++] = 0xFF;
    }
    screen_buf[BAR_START + i++] = 0xE0;

    // bottom
    i = 0;
    screen_buf[BAR_START + 16 * 13 + i++] = 0x00;
    screen_buf[BAR_START + 16 * 13 + i++] = 0x07;
    for(j = 0; j < 12; j++){
	screen_buf[BAR_START + 16 * 13 + i++] = 0xFF;
    }
    screen_buf[BAR_START + 16 * 13  + i++] = 0xE0;

    //left & right
    for(j = 0; j < 12; j++){
	screen_buf[BAR_START + j * 16 + 16 + 1] = 0x04;
	screen_buf[BAR_START + j * 16 + 16 + 14] = 0x20;
    }
    
    draw_rectangle(0, 3 * 8, 16 , 16);
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

static uint8_t read(uint32_t command){

    uint8_t lsb;
    uint8_t msb;
    uint8_t tmp;
    send_sync(command >> 8);
    msb = spi_rx() & 0xF0;
    lsb = spi_rx() & 0xF0;
    tmp = msb | (lsb >> 4);
    return tmp;
}

static void wait_busy(){

#ifdef CHECK_BUSY
    uint8_t tmp;
    do {
	tmp = read(READ_BUSY_ADDR);
	tmp &= READ_BUSY_MASK;
    }while(tmp);
    
#else
    
    for(delay = 0; delay < 1200; delay++){
	asm("");
    }
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
