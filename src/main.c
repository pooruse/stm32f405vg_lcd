#include "stm32f4xx.h"
#include "gpio.h"
#include "spi.h"


void SysTick_Handler(void);
int tick = 0;

int main(void){

    SystemCoreClockUpdate();
    
    gpio_init();
    spi_init();
    
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
