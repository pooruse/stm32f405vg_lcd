#include "stm32f4xx.h"
#include "gpio.h"
#include "st7920.h"
#include <stdio.h>


void SysTick_Handler(void);
volatile int tick = 0;
int delay;

int main(void){

    int percentage = 0;
    int increase = 1;
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
    
    printf("Ackuretta LCD Test \n\r");
    printf("number: %d\n\r", 535);
    printf("float: %1.2f\n\r",1.55);

    lcd_bar_create();
    
    SysTick_Config(SystemCoreClock/1000);
    lcd_set_font_addr(0,5);
    printf("progress: ");
    while(1){
	if(tick >= 200) {
	    tick = 0;
	    gpio_toggle_debug_led();
	    lcd_bar_set(percentage);
	    
	    printf("%3d%%\b\b\b\b", percentage);
	    if(increase == 1){
		percentage++;
		if(percentage == 100){
		    increase = 0;
		}
	    } else {
		percentage--;
		if(percentage == 0){
		    increase = 1;
		}
	    }
	    
	}
    }
}

void SysTick_Handler(void) {
    tick++;
}
