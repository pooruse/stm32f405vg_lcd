#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

void spi_init(void);
void spi_tx(uint8_t dat);
uint8_t spi_rx(void);

#endif
