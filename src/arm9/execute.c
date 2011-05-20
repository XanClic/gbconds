#include <cpu.h>
#include <mem.h>
#include <panic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define CYCLE_STEP 16
#define HALT_CYCLE_STEP 32

#define h(o, f) [o] = &__op_handler_##f
#define hf(f) static void __op_handler_##f(void)

extern const int cycles[256], cycles0xCB[256];
extern const uint16_t daa_table[2048];
extern bool interrupt_issued, main_int_flag, has_cgb;

extern uint16_t bpalette[32], opalette[32];
extern uint8_t *btm[2], *bwtd[2], *wtm[2];

union reg fr_af, fr_bc, fr_de, fr_hl, fr_sp, fr_ip;

#define r_af fr_af.f
#define r_bc fr_bc.f
#define r_de fr_de.f
#define r_hl fr_hl.f
#define r_sp fr_sp.f
#define r_ip fr_ip.f

#define r_a fr_af.hh
#define r_b fr_bc.hh
#define r_c fr_bc.lh
#define r_d fr_de.hh
#define r_e fr_de.lh
#define r_f fr_af.lh
#define r_h fr_hl.hh
#define r_l fr_hl.lh

#define nn mem_read16(r_ip)

#include <gen-operations.h>
#include "operations.h"

void execute(void)
{
    if (has_cgb)
    {
        r_ip = 0x0100;
        r_sp = 0xFFFE;
        r_af = 0x11B0;
        r_bc = 0x0000;
        r_de = 0xFF56;
        r_hl = 0x000D;
    }
    else
    {
        r_ip = 0x0100;
        r_sp = 0xFFFE;
        r_af = 0x01B0;
        r_bc = 0x0013;
        r_de = 0x00D8;
        r_hl = 0x014D;
    }

    io_regs->sb = 0xFF;
    io_regs->sc = 0x00;
    io_regs->tima = 0x00;
    io_regs->tma = 0x00;
    io_regs->tac = 0x00;
    io_regs->nr10 = 0x80;
    io_regs->nr11 = 0xBF;
    io_regs->nr12 = 0xF3;
    io_regs->nr14 = 0xBF;
    io_regs->nr21 = 0x3F;
    io_regs->nr22 = 0x00;
    io_regs->nr24 = 0xBF;
    io_regs->nr30 = 0x7F;
    io_regs->nr31 = 0xFF;
    io_regs->nr32 = 0x9F;
    io_regs->nr33 = 0xBF;
    io_regs->nr41 = 0xFF;
    io_regs->nr42 = 0x00;
    io_regs->nr43 = 0x00;
    io_regs->nr44 = 0xBF;
    io_regs->nr50 = 0x77;
    io_regs->nr51 = 0xF3;
    io_regs->nr52 = 0xF1;
    io_regs->lcdc = 0x91;
    io_regs->stat = 1;
    io_regs->scy = 0x00;
    io_regs->scx = 0x00;
    io_regs->lyc = 0x00;
    io_regs->bgp = 0xFC;
    io_regs->obp0 = 0xFF;
    io_regs->obp1 = 0xFF;
    io_regs->wy = 0x00;
    io_regs->wx = 0x00;
    io_regs->int_enable = 0x00;

    btm[0] = (uint8_t *)0x02301800;
    btm[1] = (uint8_t *)0x02303800;
    bwtd[0] = (uint8_t *)0x02300000;
    bwtd[1] = (uint8_t *)0x02302000;
    wtm[0] = (uint8_t *)0x02301800;
    wtm[1] = (uint8_t *)0x02303800;

    unsigned cycles_gone;

    for (;;)
    {
        // kprintf("%V %V %V %V %V %V\n", r_ip, r_af, r_bc, r_de, r_hl, r_sp);

        uint8_t op = mem_read8(r_ip++);

        if (handle[op] == NULL)
        {
            kprintf("[%V] Unbekannter Opcode %X.\n", r_ip - 1, op);
            gbc_panic();
        }

        handle[op]();

        cycles_gone += (op == 0xCB) ? cycles0xCB[mem_read8(r_ip)] : cycles[op];
        if (cycles_gone >= CYCLE_STEP)
        {
            update_timer(cycles_gone);
            cycles_gone = 0;

            generate_interrupts();
        }
    }
}
