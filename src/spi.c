#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "spi.h"

#define SPI1_SCK_GPIO_PORT GPIOA
#define SPI1_SCK_GPIO_PIN  GPIO_Pin_5
#define SPI1_SCK_GPIO_SRC GPIO_PinSource5
#define SPI1_MISO_GPIO_PORT GPIOA
#define SPI1_MISO_GPIO_PIN  GPIO_Pin_6
#define SPI1_MISO_GPIO_SRC GPIO_PinSource6
#define SPI1_MOSI_GPIO_PORT GPIOA
#define SPI1_MOSI_GPIO_PIN  GPIO_Pin_7
#define SPI1_MOSI_GPIO_SRC GPIO_PinSource7
#define SPI1_CS_GPIO_PORT GPIOC
#define SPI1_CS_GPIO_PIN  GPIO_Pin_4
#define SPI1_CS_GPIO_SRC GPIO_PinSource4

void spi_init(void){

    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    
    GPIO_InitStructure.GPIO_Pin = SPI1_SCK_GPIO_PIN;
    GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_GPIO_PIN;
    GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(SPI1_SCK_GPIO_PORT, SPI1_SCK_GPIO_SRC, GPIO_AF_SPI1);
    GPIO_PinAFConfig(SPI1_MOSI_GPIO_PORT, SPI1_MOSI_GPIO_SRC, GPIO_AF_SPI1);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = SPI1_CS_GPIO_PIN;
    GPIO_ResetBits(SPI1_CS_GPIO_PORT, SPI1_CS_GPIO_PIN);
    GPIO_Init(SPI1_CS_GPIO_PORT, &GPIO_InitStructure);

    SPI_DeInit(SPI1);
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    
    SPI_Cmd(SPI1, ENABLE);
}

void spi_cs(uint32_t state){
    if(state == 1) {
	GPIO_SetBits(SPI1_CS_GPIO_PORT, SPI1_CS_GPIO_PIN);    
    } else {
	GPIO_ResetBits(SPI1_CS_GPIO_PORT, SPI1_CS_GPIO_PIN);    
    }
}

void spi_tx(uint8_t dat){

    SPI_BiDirectionalLineConfig(SPI1, SPI_Direction_Tx);
    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
    SPI_SendData(SPI1, dat);
    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET);
}

uint8_t spi_rx(void){

    uint8_t dat;

    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET);
    
    SPI_BiDirectionalLineConfig(SPI1, SPI_Direction_Rx);
    SPI_SendData(SPI1, 0x00);
    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) != RESET);
    dat = SPI_ReceiveData(SPI1);
    SPI_BiDirectionalLineConfig(SPI1, SPI_Direction_Tx);

    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET);
    
    return dat;
}
