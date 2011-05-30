.arm
.extern main
.extern memcpy_fast
.global _start

_start:
/* IRQs/FIQs deaktivieren */
mrs     r0,cpsr
orr     r0,#0xC0
msr     cpsr_ctl,r0

/* MPU deaktivieren, TCMs aktivieren, Vektoren nach 0x00000000 */
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
mov     r0,#0x02000000
orr     r0,#0x007E0000
orr     r2,r0,#5<<1
mcr     15,0,r2,c9,c1,0

/* Stack */
add     r13,r0,#0x00040000

/* 32 kB WRAM */
mov     r0,#0x04000000
ldrb    r1,[r0,#0x247]
bic     r1,#0x3
strb    r1,[r0,#0x247]


/* Zugriffsrechte: Alles alles, ITCM aber nur lesbar */
/* Region 0: R/W; Region 1: R; Region 2: R/W; Region 3: R/W */
mov     r0,#0x00000063
orr     r0,#0x00003300
mcr     p15,0,r0,c5,c0,2
mcr     p15,0,r0,c5,c0,3
/* Region 0: 0x00000000 bis 0xFFFFFFFF (alles, außer den folgenden) */
mov     r0,#31 << 1 | 1
mcr     p15,0,r0,c6,c0,0
mcr     p15,0,r0,c6,c0,1
/* Region 1: 0x00000000 bis 0x01FFFFFF (ITCM) */
mov     r0,#20 << 1 | 1
mcr     p15,0,r0,c6,c1,0
mcr     p15,0,r0,c6,c1,1
/* Region 2: 0x04000000 bis 0x07FFFFFF (I/O, Palettes, VRAM, OAM) */
mov     r0,#21 << 1 | 1
orr     r0,#0x04000000
mcr     p15,0,r0,c6,c2,0
mcr     p15,0,r0,c6,c2,1
/* Region 3: 0x027E0000 bis 0x027E3FFF (DTCM) */
mov     r0,#13 << 1 | 1
orr     r0,#0x02000000
orr     r0,#0x007E0000
mcr     p15,0,r0,c6,c3,0
mcr     p15,0,r0,c6,c3,1

/* Cache für Region 0 aktivieren, für andere deaktivieren */
mov     r0,#0x01
mcr     p15,0,r0,c2,c0,0
mcr     p15,0,r0,c2,c0,1
mcr     p15,0,r0,c3,c0,0

/* MPU aktivieren */
mrc     p15,0,r0,c1,c0,0
orr     r0,#1
mcr     p15,0,r0,c1,c0,0

bl      main
b       .
