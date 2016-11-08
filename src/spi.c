#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "spi.h"

void init_spi(void){
    SPI_InitTypeDef SPI_InitStructrue;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

}
