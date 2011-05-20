#include <io.h>
#include <rom.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool has_cgb, ext_ram, batt, rtc, rmbl;
unsigned mbc, rom_size, ram_size;

#define CART_TYPE(id, mbctype, extram, battery, timer, rumble) \
    case id: \
        mbc = mbctype; \
        ext_ram = extram; \
        batt = battery; \
        rtc = timer; \
        rmbl = rumble; \
        break;

bool load_rom(void)
{
    vw16(IO, 0x204, vr16(IO, 0x204) & ~(1 << 7));

    uint8_t *rom = (uint8_t *)0x08000000;

    uint8_t check = 0;
    for (size_t off = 0x134; off <= 0x14C; off++)
        check -= rom[off] + (uint8_t)1;

    if (check != rom[0x14D])
        return false;

    if (rom[0x143] & 0x80)
        has_cgb = true;
    else
        has_cgb = false;

    switch (rom[0x147])
    {
        CART_TYPE(0x00, 0, false, false, false, false);
        CART_TYPE(0x01, 1, false, false, false, false);
        CART_TYPE(0x02, 1,  true, false, false, false);
        CART_TYPE(0x03, 1,  true,  true, false, false);
        CART_TYPE(0x05, 2, false, false, false, false);
        CART_TYPE(0x06, 2, false,  true, false, false);
        CART_TYPE(0x08, 0,  true, false, false, false);
        CART_TYPE(0x09, 0,  true,  true, false, false);
        CART_TYPE(0x0F, 3, false,  true,  true, false);
        CART_TYPE(0x10, 3,  true,  true,  true, false);
        CART_TYPE(0x11, 3, false, false, false, false);
        CART_TYPE(0x12, 3,  true, false, false, false);
        CART_TYPE(0x13, 3,  true,  true, false, false);
        CART_TYPE(0x19, 5, false, false, false, false);
        CART_TYPE(0x1A, 5,  true, false, false, false);
        CART_TYPE(0x1B, 5,  true,  true, false, false);
        CART_TYPE(0x1C, 5, false, false, false,  true);
        CART_TYPE(0x1D, 5,  true, false, false,  true);
        CART_TYPE(0x1E, 5,  true,  true, false,  true);
        default:
            return false;
    }

    rom_size = rom[0x148];
    switch (rom_size)
    {
        case 0x52:
            rom_size = 72;
            break;
        case 0x53:
            rom_size = 80;
            break;
        case 0x54:
            rom_size = 96;
            break;
        default:
            if (rom_size > 6)
                return false;
            rom_size = 2 << rom_size;
    }

    switch (rom[0x149])
    {
        case 0:
            ram_size = 0;
            break;
        case 1:
        case 2:
            ram_size = 1;
            break;
        case 3:
            ram_size = 4;
            break;
        case 4:
            ram_size = 16;
            break;
        default:
            return false;
    }

    return true;
}
