#include "stm32f4xx.h"
#include "gpio.h"
#include "st7920.h"


void SysTick_Handler(void);
int tick = 0;

int delay;
int main(void){

    SystemCoreClockUpdate();

    for(delay = 0; delay < 4000000; delay++); // delay for lcd test
    
    gpio_init();
    st7920_init();


    lcd_draw_font(0, 1*8+5, 0);
    lcd_draw_font(1, 2*8+5, 0);
    lcd_draw_font(2, 3*8+5, 0);
    lcd_draw_font(3, 4*8+5, 0);
    lcd_draw_font(4, 5*8+5, 0);
    lcd_draw_font(5, 6*8+5, 0);
    lcd_draw_font(6, 7*8+5, 0);
    
    SysTick_Config(SystemCoreClock/1000);
    
    while(1){
	if(tick >= 1000) {
	    tick = 0;
	    gpio_toggle_debug_led();
	}
    }
}

void SysTick_Handler(void) {
    tick++;
}
