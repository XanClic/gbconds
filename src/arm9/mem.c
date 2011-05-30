#include <config.h>
#include <cpu.h>
#include <mem.h>
#include <panic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

uintptr_t rom_base = 0x08000000, ram_base = 0x0A000000;

extern uintptr_t areas[16];
extern bool may_write[16];

static int cramb = 0, cromb = 1;
static bool ram_enabled = false;
static unsigned latched_time;

extern unsigned mbc;

void init_vm_memory(void)
{
    areas[0x0] = 0x08000000; // ROM-Bank 0
    areas[0x1] = 0x08001000;
    areas[0x2] = 0x08002000;
    areas[0x3] = 0x08003000;
    areas[0x4] = 0x08004000; // ROM-Bank 1
    areas[0x5] = 0x08005000;
    areas[0x6] = 0x08006000;
    areas[0x7] = 0x08007000;
    areas[0x8] = 0x02300000; // VRAM-Bank 0
    areas[0x9] = 0x02301000;
    areas[0xA] = 0x0A000000; // RAM-Bank 0
    areas[0xB] = 0x0A001000;
    areas[0xC] = 0x03000000; // WRAM-Bank 0
    areas[0xD] = 0x03001000; // WRAM-Bank 1
    areas[0xE] = 0x03000000; // WRAM-Bank 0
    areas[0xF] = 0;          // Highmem

    may_write[0x0] = may_write[0x1] = may_write[0x2] = may_write[0x3] = may_write[0x4] = may_write[0x5] = may_write[0x6] = may_write[0x7] = may_write[0xF] = false;
    may_write[0x8] = may_write[0x9] = may_write[0xA] = may_write[0xB] = may_write[0xC] = may_write[0xD] = may_write[0xE] = true;
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
                if (cramb >= 4)
                {
                    areas[0xA] = areas[0xB] = 0;
                    may_write[0xA] = may_write[0xB] = false;
                }
                else
                {
                    areas[0xA] = ram_base + cramb * 0x2000 + 0x0000;
                    areas[0xB] = ram_base + cramb * 0x2000 + 0x1000;
                    may_write[0xA] = may_write[0xB] = true;
                }
            }
            break;
        case 0x2000:
            cromb = val ? val : 1;
            areas[4] = rom_base + cromb * 0x4000 + 0x0000;
            areas[5] = rom_base + cromb * 0x4000 + 0x1000;
            areas[6] = rom_base + cromb * 0x4000 + 0x2000;
            areas[7] = rom_base + cromb * 0x4000 + 0x3000;
            break;
        case 0x4000:
            if (val >= 8)
            {
                cramb = val;
                areas[0xA] = areas[0xB] = 0;
                may_write[0xA] = may_write[0xB] = false;
            }
            else if (val < 4)
            {
                cramb = val;
                if (ram_enabled)
                {
                    areas[0xA] = ram_base + cramb * 0x2000 + 0x0000;
                    areas[0xB] = ram_base + cramb * 0x2000 + 0x1000;
                    may_write[0xA] = may_write[0xB] = true;
                }
                else
                {
                    areas[0xA] = areas[0xB] = 0;
                    may_write[0xA] = may_write[0xB] = false;
                }
            }
            else
            {
                areas[0xA] = areas[0xB] = 0;
                may_write[0xA] = may_write[0xB] = false;
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

uint8_t eram_read8(unsigned off)
{
    if (ram_read[mbc] == NULL)
    {
        kprintf("[mem] RAM-Lesezugriff ohne RAM von 0x%V.\n", off);
        gbc_panic();
    }

    return ram_read[mbc](off);
}

void eram_write8(unsigned off, uint8_t val)
{
    if (ram_write[mbc] == NULL)
    {
        kprintf("[mem] RAM-Schreibzugriff ohne RAM von 0x%X auf 0x%V.\n", val, off);
        gbc_panic();
    }

    ram_write[mbc](off, val);
}

void rom_write8(unsigned off, uint8_t val)
{
#ifdef GBC_CARTRIDGE
    // Bei einer echten Cartridge macht das der GBC
    *(uint8_t *)(rom_base + off) = val;
#else
    if (rom_write[mbc] == NULL)
    {
        kprintf("[mem] ROM-Schreibzugriff (0x%X) auf 0x%V.\n", val, off);
        gbc_panic();
    }

    rom_write[mbc](off, val);
#endif
}

uint8_t mem_read8(uint16_t addr)
{
    unsigned ix = addr >> 12;
    addr &= 0xFFF;

    if (areas[ix])
        return *(uint8_t *)(areas[ix] + addr);

    if (ix != 15)
        return eram_read8(addr | ((ix & 1) << 12));
    else
    {
        if (addr < 0xE00)
            return *(uint8_t *)(areas[0xD] + addr);
        return *(uint8_t *)(DTCM + (addr & 0x1FF)); // OAM/HRAM/IO
    }
}

uint16_t mem_read16(uint16_t addr)
{
    if (addr & 1)
        return mem_read8(addr) | (mem_read8(addr + 1) << 8);

    unsigned ix = addr >> 12;
    addr &= 0xFFF;

    if (areas[ix])
        return *(uint16_t *)(areas[ix] + addr);

    if (ix != 15)
    {
        addr |= (ix & 1) << 12;
        return eram_read8(addr) | (eram_read8(addr + 1) << 8);
    }
    else
    {
        if (addr < 0xE00)
            return *(uint16_t *)(areas[0xD] + addr);
        return *(uint16_t *)(DTCM + (addr & 0x1FF));
    }
}

void mem_write8(uint16_t addr, uint8_t val)
{
    unsigned ix = addr >> 12;

    if (may_write[ix])
    {
        *(uint8_t *)(areas[ix] + (addr & 0xFFF)) = val;
        return;
    }

    if (ix < 8)
    {
        rom_write8(addr, val);
        return;
    }
    else if (ix < 12)
    {
        eram_write8(addr - 0xA000, val);
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

    if (ix < 8)
    {
        rom_write8(addr, val & 0xFF);
        rom_write8(addr + 1, val >> 8);
        return;
    }
    else if (ix < 12)
    {
        eram_write8(addr - 0xA000, val & 0xFF);
        eram_write8(addr - 0x9FFF, val >> 8);
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
