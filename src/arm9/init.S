.arm
.extern main
.extern memcpy_fast
.global _start

_start:
/* IRQs/FIQs deaktivieren */
mrs     r0,cpsr
orr     r0,#0xC0
msr     cpsr_ctl,r0

/* PU deaktivieren, TCMs aktivieren, Vektoren nach 0x00000000 */
mrc     15,0,r0,c1,c0,0
bic     r0,#0x00005
bic     r0,#0xAA000
orr     r0,#0x00074
orr     r0,#0x51000
mcr     15,0,r0,c1,c0,0

/* Caches invalidieren */
mov     r1,#0
mcr     15,0,r1,c7,c5,0
mcr     15,0,r1,c7,c6,0
mcr     15,0,r1,c7,c10,4

/* TCMs an 0x00000000 bzw. 0x027E0000 legen */
mov     r0,#16<<1
mcr     15,0,r0,c9,c1,1
mov     r0,   #0x02000000
orr     r2,r0,#0x007E0000
orr     r2,#5<<1
mcr     15,0,r2,c9,c1,0

/* 32 kB WRAM */
mov     r0,#0x04000000
ldrb    r1,[r0,#0x247]
bic     r1,#0x3
strb    r1,[r0,#0x247]

bl      main
b       .
