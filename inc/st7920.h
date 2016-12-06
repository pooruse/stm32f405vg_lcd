#ifndef _ST7920_H
#define _ST7020_H

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

#include <stdint.h>

enum {
    LCD_LINE_0,
    LCD_LINE_1,
    LCD_LINE_2,
    LCD_LINE_3,
    LCD_LINE_4,
    LCD_LINE_5,
    LCD_LINE_6,
    LCD_LINE_7,
    LCD_LINE_OFF,
};


struct st7920_draw_rectangle_t
{
    uint8_t *buf; 
    int size; // buffer byte size
    int x; // range: 0~128
    int y; // range: 0~64
    int w; // width range: 1~128
    int h; // height 1~64
};

void st7920_init(void);
void lcd_draw_rectangle(struct st7920_draw_rectangle_t draw);
void lcd_clear(void);
void lcd_set_font_addr(int x, int y);
void lcd_putc(char c);
void lcd_bar_set(int value);
void lcd_bar_create(void);
void lcd_select_line(int n);

void lcd_printf(const char *format,...);

#endif

