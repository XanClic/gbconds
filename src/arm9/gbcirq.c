#include <cpu.h>
#include <stdbool.h>

bool interrupt_issued = false, main_int_flag = false;

extern union reg fr_ip;

void generate_interrupts(void)
{
    int cause = io_regs->int_enable & io_regs->int_flag;

    if (!main_int_flag || !cause)
        return;

    main_int_flag = false;
    push(fr_ip.f);

    if (cause & INT_P10_P13) // Hi-Lo of P10-P13
        cause = 4;
    else if (cause & INT_SERIAL) // Serial transfer completed
        cause = 3;
    else if (cause & INT_TIMER) // Timer overflow
        cause = 2;
    else if (cause & INT_LCDC_STAT) // LCDC STAT
        cause = 1;
    else if (cause & INT_VBLANK) // V Blank
        cause = 0;
    else
        return;

    interrupt_issued = true;

    io_regs->int_flag &= ~(1 << cause);

    fr_ip.f = 0x40 + cause * 8;
}
