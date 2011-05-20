#include <config.h>
#include <cpu.h>
#include <mem.h>
#include <panic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef REAL_CARTRIDGE
#define RAM_BASE 0x02200000
#else
#define RAM_BASE 0x0A000000
#endif

uintptr_t areas[16] = {
    0x08000000, // ROM-Bank 0
    0x08001000,
    0x08002000,
    0x08003000,
    0x08004000, // ROM-Bank 1
    0x08005000,
    0x08006000,
    0x08007000,
    0x02300000, // VRAM-Bank 0
    0x02301000,
    RAM_BASE + 0x0000, // RAM-Bank 0
    RAM_BASE + 0x1000,
    0x03000000, // WRAM-Bank 0
    0x03001000, // WRAM-Bank 1
    0x03000000, // WRAM-Bank 0
    0, // Highmem
};

bool may_write[16] = {
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    false
};

static int cramb = 0, cromb = 1;
static bool ram_enabled = false, ram_mapped;
static unsigned latched_time;

extern unsigned mbc;

uint8_t mem_read8(uint16_t addr)
{
    unsigned ix = addr >> 12;
    addr &= 0xFFF;

    if (areas[ix])
        return *(uint8_t *)(areas[ix] + addr);

    if (addr < 0xE00)
        return *(uint8_t *)(areas[0xD] + addr);

    return *(uint8_t *)(DTCM + (addr & 0x1FF)); // OAM/HRAM/IO
}

uint16_t mem_read16(uint16_t addr)
{
    if (addr & 1)
        return mem_read8(addr) | (mem_read8(addr + 1) << 8);

    unsigned ix = addr >> 12;
    addr &= 0xFFF;

    if (areas[ix])
        return *(uint16_t *)(areas[ix] + addr);

    if (addr < 0xE00)
        return *(uint16_t *)(areas[0xD] + addr);

    return *(uint16_t *)(DTCM + (addr & 0x1FF));
}

static void mbc3_rom_write(unsigned off, uint8_t val)
{
    static unsigned latched_state = 1;

    switch (off & 0x6000)
    {
        case 0x0000:
            if (!val)
                ram_enabled = false;
            else if (val == 0x0A)
            {
                ram_enabled = true;
                if ((cramb < 4) && !ram_mapped)
                {
                    areas[0xA] = RAM_BASE + cramb * 0x2000;
                    areas[0xB] = RAM_BASE + cramb * 0x2000 + 0x1000;
                }
            }
            break;
        case 0x2000:
#ifdef REAL_CARTRIDGE
            // Bei einer echten GBC-Cartridge wäre der MBC integriert
            *(uint8_t *)0x08002000 = val;
#else
            cromb = val ? val : 1;
            areas[4] = 0x08000000 + cromb * 0x4000;
            areas[5] = 0x08001000 + cromb * 0x4000;
            areas[6] = 0x08002000 + cromb * 0x4000;
            areas[7] = 0x08003000 + cromb * 0x4000;
#endif
            break;
        case 0x4000:
            if (val >= 8)
                cramb = val;
            else if ((val < 4) && (cramb != val))
            {
                cramb = val;
                if (ram_enabled)
                {
                    areas[0xA] = RAM_BASE + cramb * 0x2000;
                    areas[0xB] = RAM_BASE + cramb * 0x2000 + 0x1000;
                }
            }
            break;
        case 0x6000:
            if (!val)
                latched_state = 0;
            else if ((val == 1) && (!latched_state))
            {
                latched_state = 1;
                latched_time = 0; // TODO
            }
            break;
    }
}

static void mbc3_ram_write(unsigned off, uint8_t val)
{
    if ((cramb < 8) || (cramb > 12))
    {
        kprintf("[mbc3] Fehlgeschlagener RAM-Schreibzugriff (0x%X nach 0x%V) auf Bank %i.\n", val, off, cramb);
        gbc_panic();
    }

    if (!ram_enabled)
    {
        kputs("[mbc3] Versuch, nicht aktivierten RAM zu beschreiben.\n");
        return;
    }

    kprintf("[mbc3] Ignoriere geschriebenen Zeitwert 0x%X ins Register %i.\n", val, cramb);
}

static uint8_t mbc3_ram_read(unsigned off)
{
    if (!ram_enabled)
    {
        kprintf("[mbc3] Versuch, von nicht aktiviertem RAM zu lesen.\n");
        return 0;
    }

    switch (cramb)
    {
        case 8:
            return 0;
        case 9:
            return 0;
        case 10:
            return 0;
        case 11:
            return 0;
        case 12:
            return 0;
        default:
            kprintf("[mbc3] Fehlgeschlagener RAM-Lesezugriff von 0x%V auf Bank %i.\n", off, cramb);
            gbc_panic();
    }
}

static void (*const rom_write[6])(unsigned off, uint8_t val) = {
    [3] = &mbc3_rom_write
};

static uint8_t (*const ram_read[6])(unsigned off) = {
    [3] = &mbc3_ram_read
};

static void (*const ram_write[6])(unsigned off, uint8_t val) = {
    [3] = &mbc3_ram_write
};

void rom_write8(unsigned off, uint8_t val)
{
    if (rom_write[mbc] == NULL)
    {
        kprintf("[mem] ROM-Schreibzugriff (0x%X) auf 0x%V.\n", val, off);
        gbc_panic();
    }

    rom_write[mbc](off, val);
}

void mem_write8(uint16_t addr, uint8_t val)
{
    unsigned ix = addr >> 12;

    if (may_write[ix])
    {
        *(uint8_t *)(areas[ix] + (addr & 0xFFF)) = val;
        return;
    }

    if (addr < 0x8000)
    {
        rom_write8(addr, val);
        return;
    }

    addr &= 0xFFF;

    if (addr < 0xE00)
        *(uint8_t *)(areas[0xD] + addr) = val;
    else if ((addr >= 0xF00) && (addr < 0xF80))
        io_out8(addr & 0xFF, val);
    else
        *(uint8_t *)(DTCM + (addr & 0x1FF)) = val;
}

void mem_write16(uint16_t addr, uint16_t val)
{
    if (addr & 1)
    {
        mem_write8(addr, val & 0xFF);
        mem_write8(addr + 1, val >> 8);
        return;
    }

    unsigned ix = addr >> 12;

    if (may_write[ix])
    {
        *(uint16_t *)(areas[ix] + (addr & 0xFFF)) = val;
        return;
    }

    if (addr < 0x8000)
    {
        rom_write8(addr, val & 0xFF);
        rom_write8(addr + 1, val >> 8);
        return;
    }

    addr &= 0xFFF;

    if (addr < 0xE00)
        *(uint16_t *)(areas[0xD] + addr) = val;
    else if ((addr >= 0xF00) && (addr < 0xF80))
    {
        io_out8( addr      & 0xFF, val & 0xFF);
        io_out8((addr + 1) & 0xFF, val >> 8);
    }
    else
        *(uint16_t *)(DTCM + (addr & 0x1FF)) = val;
}
