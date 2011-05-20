#include <cpu.h>
#include <gbclcd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern bool lcd_on, hdma_on;

static const int collect_overflow[4] =
{
    256, //   4096 Hz
      4, // 262144 Hz
     16, //  65536 Hz
     64  //  16384 Hz
};

extern uintptr_t areas[16];

void hdma_copy_16b(void)
{
    uint16_t hdma_src = (io_regs->hdma1 << 8) | io_regs->hdma2;
    uint16_t hdma_dst = (io_regs->hdma3 << 8) | io_regs->hdma4;

    memcpy((void *)(areas[hdma_dst >> 12] + (hdma_dst & 0xFFF)), (void *)(areas[hdma_src >> 12] + (hdma_src & 0xFFF)), 16);

    hdma_src += 16;
    hdma_dst += 16;

    io_regs->hdma1 = hdma_src >> 8;
    io_regs->hdma2 = hdma_src & 0xFF;
    io_regs->hdma3 = hdma_dst >> 8;
    io_regs->hdma4 = hdma_dst & 0xFF;

    if (--io_regs->hdma5 & 0x80)
        hdma_on = 0;

    update_timer(8);
    if (hdma_on)
        generate_interrupts();
}

void update_timer(unsigned cycles_gone)
{
    static int collected = 0, vsync_collect = 0, div_collect = 0, redrawed = 0;
    int hblank_start = 0;

    div_collect += cycles_gone;
    while (div_collect >= 64)
    {
        io_regs->div++;
        div_collect -= cycles_gone;
    }

    if (lcd_on)
    {
        vsync_collect += cycles_gone;

        io_regs->stat &= ~3;
        if (io_regs->ly >= 144)
            io_regs->stat |= 1;
        else
        {
            if (vsync_collect < 51)
                redrawed = 0;
            else if (vsync_collect < 71)
                io_regs->stat |= 2;
            else
            {
                io_regs->stat |= 3;
                if (!redrawed)
                {
                    draw_line(io_regs->ly);
                    redrawed = 1;
                }
            }
        }

        while (vsync_collect >= 114) // Eine Zeile wäre jetzt fertig
        {
            vsync_collect -= 114;

            hblank_start = 1;
            if (hdma_on)
                hdma_copy_16b();

            if (++io_regs->ly > 153)
                io_regs->ly = 0;
            if (io_regs->ly == 144)
            {
                io_regs->stat &= ~3;
                io_regs->stat |= 1;
                io_regs->int_flag |= INT_VBLANK;
            }

            if (io_regs->lyc == io_regs->ly)
            {
                io_regs->stat |= (1 << 2);
                if (io_regs->stat & (1 << 6))
                    io_regs->int_flag |= INT_LCDC_STAT;
            }
            else
                io_regs->stat &= ~(1 << 2);
        }

        if ((io_regs->stat & (1 << 5)) && ((io_regs->stat & 3) == 2))
            io_regs->int_flag |= INT_LCDC_STAT;
        else if ((io_regs->stat & (1 << 4)) && ((io_regs->stat & 3) == 1))
            io_regs->int_flag |= INT_LCDC_STAT;
        else if ((io_regs->stat & (1 << 3)) && hblank_start)
            io_regs->int_flag |= INT_LCDC_STAT;
    }

    if (!(io_regs->tac & (1 << 2)))
        return;

    collected += cycles_gone;
    while (collected >= collect_overflow[io_regs->tac & 3])
    {
        collected -= collect_overflow[io_regs->tac & 3];
        if (!++io_regs->tima)
        {
            io_regs->tima = io_regs->tma;
            io_regs->int_flag |= INT_TIMER;
        }
    }
}
