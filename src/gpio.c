#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "gpio.h"

#define DEBUG_LED_PORT GPIOC
#define DEBUG_LED_PIN GPIO_Pin_3

void gpio_init(void){
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOJ, ENABLE);
    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = DEBUG_LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    
    GPIO_Init(DEBUG_LED_PORT, &GPIO_InitStructure);
}

void gpio_set_debug_led(void){
    GPIO_SetBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
}

void gpio_reset_debug_led(void){
    GPIO_ResetBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
}

void gpio_toggle_debug_led(void){
    GPIO_ToggleBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
}
