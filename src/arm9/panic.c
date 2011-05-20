#include <cpu.h>
#include <panic.h>
#include <stdio.h>

extern union reg fr_af, fr_bc, fr_de, fr_hl, fr_sp, fr_ip;

void panic(void)
{
    // FIXME: Was könnte man hier machen? Vermutlich am besten dem ARM7 per IPC stilllegen und was hübsches ausgeben.
    for (;;);
}

void gbc_panic(void)
{
    kputs("Ausführung abgebrochen:\n");
    kprintf("AF=%V BC=%V DE=%V HL=%V\n", fr_af.f, fr_bc.f, fr_de.f, fr_hl.f);
    kprintf("SP=%V IP=%V\n", fr_sp.f, fr_ip.f);

    for (;;);
}

int raise(int sig)
{
    (void)sig;
    panic();
}
