#ifndef _VIDEO_H
#define _VIDEO_H

#include <stdint.h>

typedef uint_fast16_t rgb_t;

void cls(rgb_t color);
void display_bitmap(const void *base);
void init_video(void);
void term_putc(unsigned x, unsigned y, unsigned unicode);

#define rgb24to15(r, g, b) ((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | 0x8000)

#endif

