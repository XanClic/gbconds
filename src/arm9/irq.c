#include <io.h>
#include <string.h>

static void got_irq(void) __attribute__((interrupt("IRQ")));

static inline void ack_irqs(void)
{
    vw32(IO, 0x0214, vr32(IO, 0x0214));
}

void disable_all_irqs(void)
{
    vw16(IO, 0x0208, 0);
    vw32(IO, 0x0210, 0);

    ack_irqs();
}

void set_irq_handler(void)
{
    memsetptr((void *)0x00000000, 0xEAFFFFFE, 8);
}

void enable_irqs(void)
{
    vw16(IO, 0x0208, 1);

    __asm__ __volatile__ ("mrs r0,cpsr; bic r0,#0x80; msr cpsr_ctl,r0" ::: "r0");
}

static void __attribute__((interrupt("IRQ"))) got_irq(void)
{
    ack_irqs();
}

