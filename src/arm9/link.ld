OUTPUT_ARCH("armv5t")
ENTRY(_start)

SECTIONS
{
    .text 0x02000000 :
    {
        *(.text)
    }
    .data :
    {
        . = ALIGN(4);
        *(.data)
    }
    .bss :
    {
        . = ALIGN(4);
        *(.bss)
    }
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
    }

    fr_af = 0x027E1000;
    fr_bc = 0x027E1004;
    fr_de = 0x027E1008;
    fr_hl = 0x027E100C;
    fr_sp = 0x027E1010;
    fr_ip = 0x027E1014;
    btm = 0x027E1018;
    bwtd = 0x027E1020;
    wtm = 0x027E1028;
    lcd_on = 0x027E1030;
    has_cgb = 0x027E1034;
    bpalette = 0x027E1040;
    opalette = 0x027E1080;
    frameskip_skip = 0x027E10C0;
    frameskip_draw = 0x027E10C4;
    frameskip_draw_i = 0x027E10C8;
    frameskip_skip_i = 0x027E10CC;
    skip_this = 0x027E10D0;
    hdma_on = 0x027E10D4;
    areas = 0x027E1100;
    may_write = 0x027E1140;
    cycles = 0x027E1800;
    cycles0xCB = 0x027E1900;
}
