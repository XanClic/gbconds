#ifndef _GBCIO_H
#define _GBCIO_H

#include <stdint.h>

struct io
{
    uint8_t p1;
    uint8_t sb;
    uint8_t sc;
    uint8_t rsvd1;
    uint8_t div;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;
    uint8_t rsvd2[7];
    uint8_t int_flag;
    uint8_t nr10;
    uint8_t nr11;
    uint8_t nr12;
    uint8_t nr13;
    uint8_t nr14;
    uint8_t rsvd3;
    uint8_t nr21;
    uint8_t nr22;
    uint8_t nr23;
    uint8_t nr24;
    uint8_t nr30;
    uint8_t nr31;
    uint8_t nr32;
    uint8_t nr33;
    uint8_t nr34;
    uint8_t rsvd4;
    uint8_t nr41;
    uint8_t nr42;
    uint8_t nr43;
    uint8_t nr44;
    uint8_t nr50;
    uint8_t nr51;
    uint8_t nr52;
    uint8_t rsvd5[9];
    uint8_t wave_pat[16];
    uint8_t lcdc;
    uint8_t stat;
    uint8_t scy;
    uint8_t scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t dma;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wy;
    uint8_t wx;
    uint8_t rsvd6;
    uint8_t key1;
    uint8_t rsvd7;
    uint8_t vbk;
    uint8_t rsvd8;
    uint8_t hdma1, hdma2, hdma3, hdma4, hdma5;
    uint8_t rp;
    uint8_t rsvd9[17];
    uint8_t bcps;
    uint8_t bcpd;
    uint8_t ocps;
    uint8_t ocpd;
    uint8_t rsvd10[4];
    uint8_t svbk;
    uint8_t rsvd11[142];
    uint8_t int_enable;
} __attribute__((packed));

#endif
