#include "stm32f4xx.h"
#include "gpio.h"
#include "spi.h"
#include "st7920.h"
#include <stdio.h>

//#define SPI_BIRX_TEST

void SysTick_Handler(void);
volatile int tick = 0;
int delay;

int main(void){
    int line_index = 0;

    SystemCoreClockUpdate();
    
    for(delay = 0; delay < 4000000; delay++){
	asm("");
    }
    
    gpio_init();
    st7920_init();
    
    lcd_clear();

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    #ifdef SPI_BIRX_TEST
    while(1){
	uint8_t tmp;
	int i;
	for(i = 0; i < 8000000; i++);
	gpio_toggle_debug_led();
	lcd_set_font_addr(0,0);
	tmp = spi_rx();
	lcd_printf("spi_rx = %X",tmp);
    }
    #endif

    /*
    lcd_printf("Ackuretta LCD Test \n\r");
    lcd_printf("number: %d\n\r", 535);
    lcd_printf("curing time: %1.2f\n\r",32.999);
    lcd_printf("layers: %d\n\r",500);
    lcd_printf("description:\n\r");
    lcd_printf("description1\n\r");
    lcd_printf("description2\n\r");
    lcd_printf("description3\n\r");
    lcd_printf("description4\n\r");
    */
    

    SysTick_Config(SystemCoreClock/1000);
    while(1){
	if(tick >= 1000) {
	    tick = 0;
	    gpio_toggle_debug_led();
	    lcd_select_line(line_index);
	    line_index++;
	    if(line_index > LCD_LINE_OFF){
		line_index = 0;
	    }
	}
    }
}

void SysTick_Handler(void) {
    tick++;
}
