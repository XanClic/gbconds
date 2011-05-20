#include <io.h>
#include <stdint.h>

static void spi_wait(void)
{
    while (vr8(IO, 0x1C0) & 0x80);
}

void init_spi(void)
{
    vw16(IO, 0x1C0, 0x8201); // Touchscreen
}

void spi_write(uint_fast8_t value)
{
    spi_wait();
    vw8(IO, 0x1C2, value);
}

uint_fast8_t spi_read(void)
{
    spi_wait();
    spi_write(0);
    spi_wait();

    return vr8(IO, 0x1C2);
}

