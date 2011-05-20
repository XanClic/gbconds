#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

void init_spi(void);
uint_fast8_t spi_read(void);
void spi_write(uint_fast8_t val);

#endif

