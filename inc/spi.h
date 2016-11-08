#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

void spi_init(void);
void spi_tx_rx(uint8_t *tx, uint32_t *rx, uint32_t size);
    
#endif
