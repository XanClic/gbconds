#include <spi.h>

void init_touchscreen(void)
{
    init_spi();
}

int touchscreen_raw_x(void)
{
    spi_write(0xD0);

    int fval = spi_read();
    return (fval << 5) | (spi_read() >> 3);
}

int touchscreen_raw_y(void)
{
    spi_write(0x90);

    int fval = spi_read();
    return (fval << 5) | (spi_read() >> 3);
}

