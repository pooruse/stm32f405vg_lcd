#include "stm32f4xx.h"
#include "gpio.h"
#include "st7920.h"
#include <stdio.h>


void SysTick_Handler(void);
int tick = 0;

int delay;
int main(void){

    SystemCoreClockUpdate();

    for(delay = 0; delay < 4000000; delay++); // delay for lcd test
    
    gpio_init();
    st7920_init();
    
    lcd_clear();

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    printf("test");
    
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
