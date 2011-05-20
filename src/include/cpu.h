#ifndef _CPU_H
#define _CPU_H

#include <config.h>
#include <gbcio.h>
#include <mem.h>
#include <stdint.h>

#define FS_ZERO   7
#define FS_SUB    6
#define FS_HCRY   5
#define FS_CRY    4
#define FLAG_ZERO (1U << FS_ZERO)
#define FLAG_SUB  (1U << FS_SUB)
#define FLAG_HCRY (1U << FS_HCRY)
#define FLAG_CRY  (1U << FS_CRY)

#define INT_P10_P13   (1 << 4)
#define INT_SERIAL    (1 << 3)
#define INT_TIMER     (1 << 2)
#define INT_LCDC_STAT (1 << 1)
#define INT_VBLANK    (1 << 0)

#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_SELECT (1 << 2)
#define KEY_START  (1 << 3)

#define KEY_RIGHT  (1 << 0)
#define KEY_LEFT   (1 << 1)
#define KEY_UP     (1 << 2)
#define KEY_DOWN   (1 << 3)

#define KEY_DIR (1 << 4)
#define KEY_OTH (1 << 5)

union reg
{
    struct
    {
        uint8_t lh, hh;
    } __attribute__((packed));
    uint16_t f;
};

#define io_regs ((struct io *)(DTCM + 0x100))

void execute(void);
void update_timer(unsigned cycles);
void generate_interrupts(void);
void io_out8(uint8_t reg, uint8_t val);


extern union reg fr_sp;

static inline void push(uint16_t val)
{
    fr_sp.f -= 2;
    mem_write16(fr_sp.f, val);
}

static inline uint16_t pop(void)
{
    uint16_t val = mem_read16(fr_sp.f);
    fr_sp.f += 2;
    return val;
}

#endif
