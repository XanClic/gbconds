#include <cpu.h>
#include <gbclcd.h>
#include <io.h>
#include <ipc.h>
#include <irq.h>
#include <mem.h>
#include <rom.h>
#include <stdint.h>
#include <stdio.h>
#include <video.h>

extern unsigned rom_size, ram_size;
extern uintptr_t rom_base, ram_base;
extern union reg fr_af, fr_bc, fr_de, fr_hl, fr_sp, fr_ip;

int main(void)
{
    disable_all_irqs();
    set_irq_handler();

    init_video();

    ipc_init();
    ipc_sync();

    init_vm_memory();

    kputs("GBConDS initialisiert.\n");

    kputs("Suche nach Game-Boy-ROM... ");

    if (!load_rom())
    {
        kputs("Nicht gefunden.\n");
        return 1;
    }

    kputs("OK\n");

    kprintf("(%i kB ROM ab %p, %i kB RAM ab %p)\n", rom_size * 16, rom_base, ram_size * 2, ram_base);

    kputs("Starte Ausführung.\n");

    init_gbc_lcd();
    execute();

    kputs("Ausführung beendet:\n");
    kprintf("AF=%V BC=%V DE=%V HL=%V\n", fr_af.f, fr_bc.f, fr_de.f, fr_hl.f);
    kprintf("SP=%V IP=%V\n", fr_sp.f, fr_ip.f);

    return 0;
}
