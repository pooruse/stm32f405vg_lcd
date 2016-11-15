#include "stm32f4xx.h"
#include "gpio.h"
#include "st7920.h"
#include <stdio.h>


void SysTick_Handler(void);
int tick = 0;
int delay;

int main(void){

    int percentage = 0;
    
    SystemCoreClockUpdate();

    for(delay = 0; delay < 4000000; delay++); // delay for lcd test
    
    gpio_init();
    st7920_init();
    
    lcd_clear();

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    //lcd_set_font_addr(0,1);
    printf("Ackuretta LCD Test\n\r");
    printf("number: %d\n\r", 535);
    printf("Hex: %X\n\r", 15);
    printf("soft float: %d.%d\n\r", 15,1234);
    printf("hard float: %1.2f\n\r",0.1234);
    printf("float: %1.2f\n\r",1.55);

    //lcd_bar_create();
    
    SysTick_Config(SystemCoreClock/1000);
    
    while(1){
	if(tick >= 1000) {
	    tick = 0;
	    gpio_toggle_debug_led();
	    lcd_bar_set(percentage);

	    percentage++;
	    if(percentage == 100){
		percentage = 0;
	    }
	}
    }
}

void SysTick_Handler(void) {
    tick++;
}
