#include <config.h>
#include <cpu.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

bool lcd_on = true;
uint16_t bpalette[32], opalette[32];
uint8_t *btm[2] = { NULL }, *bwtd[2] = { NULL }, *wtm[2] = { NULL };

extern bool has_cgb;

// #define DISPLAY_ALL

int frameskip_skip = 3, frameskip_draw = 1;
static int frameskip_draw_i = 0, frameskip_skip_i = 0, skip_this = 0;

static void draw_bg_line(int line, int o_line, int bit7val, int window)
{
    int by = line & 0xF8, ry = line & 0x07;
    int tile = by * 4;
    int sx = io_regs->scx;

    line *= 256;
    o_line *= 256;

    for (int bx = 0; bx < 256; bx += 8)
    {
        if (window && (io_regs->wx <= bx))
            break;

        int flags;
        if (has_cgb)
            flags = btm[1][tile];
        else
            flags = 0;

        if ((flags & (1 << 7)) != bit7val)
        {
            tile++;
            continue;
        }

        uint8_t *tdat;
        int vbank;
        uint16_t *pal;
        if (has_cgb)
        {
            vbank = !!(flags & (1 << 3));
            pal = &bpalette[(flags & 7) * 4];
        }
        else
        {
            vbank = 0;
            pal = bpalette;
        }

        if (bwtd[0] == (uint8_t *)0x02300000)
            tdat = &bwtd[vbank][(unsigned)btm[0][tile] * 16];
        else
            tdat = &bwtd[vbank][(int)(int8_t)btm[0][tile] * 16];

        int b1, b2;

        if (flags & (1 << 6))
        {
            b1 = tdat[(7 - ry) * 2];
            b2 = tdat[(7 - ry) * 2 + 1];
        }
        else
        {
            b1 = tdat[ry * 2];
            b2 = tdat[ry * 2 + 1];
        }

        for (int rx = 0; rx < 8; rx++)
        {
            int val, mask;

            if (flags & (1 << 5))
                mask = 1 << rx;
            else
                mask = 1 << (7 - rx);

            val = !!(b1 & mask) + !!(b2 & mask) * 2;

            if (((bx + rx - sx) & 0xFF) < 160)
                ((uint16_t *)VRAM_1)[o_line + ((bx + rx - sx) & 0xFF)] = pal[val] | (!val << 15);
        }

        tile++;
    }
}

void draw_line(int line)
{
    if (!lcd_on)
    {
        /*if (line == 143)
            os_handle_events();*/
        return;
    }

    if (skip_this && (line < 143))
        return;
    else if (line == 143)
    {
        if (skip_this)
        {
            if (++frameskip_skip_i >= frameskip_skip)
            {
                frameskip_skip_i = 0;
                skip_this = 0;
                return;
            }
        }
        else
        {
            if (++frameskip_draw_i >= frameskip_draw)
            {
                frameskip_draw_i = 0;
                if (frameskip_skip)
                    skip_this = 1;
            }
        }
    }

    struct
    {
        uint8_t y, x;
        uint8_t num;
        uint8_t flags;
    } __attribute__((packed)) *oam = (void *)DTCM;
    int sx = io_regs->scx, sy = io_regs->scy;
    int abs_line = (line + sy) & 0xFF;
    int window_active = ((io_regs->lcdc & (1 << 5)) && (io_regs->wx >= 7) && (io_regs->wx <= 166) && (io_regs->wy <= line));

    if (!(io_regs->lcdc & (1 << 0)))
        memset((uint16_t *)VRAM_1 + abs_line * 160, 0, 160 * 2);
    else
        draw_bg_line(abs_line, line, 0 << 7, window_active);

    if (window_active)
    {
        int wx = io_regs->wx + sx - 7, wy = io_regs->wy;
        int yoff = line - wy;
        int by = yoff & 0xF8, ry = yoff & 0x07;
        int tile = by * 4;

        for (int bx = 0; bx < 160; bx += 8)
        {
            int flags;
            if (has_cgb)
                flags = wtm[1][tile];
            else
                flags = 0;

            uint8_t *tdat;
            int vbank;
            uint16_t *pal;
            if (has_cgb)
            {
                vbank = !!(flags & (1 << 3));
                pal = &bpalette[(flags & 7) * 4];
            }
            else
            {
                vbank = 0;
                pal = bpalette;
            }

            if (bwtd[0] == (uint8_t *)0x02300000)
                tdat = &bwtd[vbank][(unsigned)wtm[0][tile] * 16];
            else
                tdat = &bwtd[vbank][(int)(int8_t)wtm[0][tile] * 16];

            for (int rx = 0; rx < 8; rx++)
            {
                int val;

                switch (flags & (3 << 5))
                {
                    case (0 << 5):
                        val = !!(tdat[ry * 2] & (1 << (7 - rx)));
                        val += !!(tdat[ry * 2 + 1] & (1 << (7 - rx))) << 1;
                        break;
                    case (1 << 5):
                        val = !!(tdat[ry * 2] & (1 << rx));
                        val += !!(tdat[ry * 2 + 1] & (1 << rx)) << 1;
                        break;
                    case (2 << 5):
                        val = !!(tdat[(7 - ry) * 2] & (1 << (7 - rx)));
                        val += !!(tdat[(7 - ry) * 2 + 1] & (1 << (7 - rx))) << 1;
                        break;
                    default:
                        val = !!(tdat[(7 - ry) * 2] & (1 << rx));
                        val += !!(tdat[(7 - ry) * 2 + 1] & (1 << rx)) << 1;
                }

                if (((bx + rx + wx) & 0xFF) < 160)
                    ((uint16_t *)VRAM_1)[line * 256 + ((bx + rx + wx) & 0xFF)] = pal[val];
            }

            tile++;
        }
    }

    if (io_regs->lcdc & (1 << 1))
    {
        int obj_height = (io_regs->lcdc & (1 << 2)) ? 16 : 8;
        int count = 0;

        for (int sprite = 40; sprite >= 0; sprite--)
        {
            uint8_t *tdat;
            int bx = oam[sprite].x, by = oam[sprite].y, flags = oam[sprite].flags;
            uint16_t *pal;
            if (has_cgb)
                pal = &opalette[(flags & 7) * 4];
            else
                pal = &opalette[(flags & (1 << 4)) >> 2];

            if (!has_cgb)
                flags &= 0xF0;

            bx -= 8;
            by -= 16;

            if ((by > line) || (by + obj_height <= line))
                continue;

            if (count++ >= 10)
                break;

            if ((bx <= -8) || (bx >= 160))
                continue;

            int ry = line - by;

            if (flags & (1 << 6))
                ry = (obj_height - 1) - ry;

            if (obj_height == 8)
            {
                if (!(flags & (1 << 3)))
                    tdat = (uint8_t *)0x02300000 + oam[sprite].num * 16;
                else
                    tdat = (uint8_t *)0x02302000 + oam[sprite].num * 16;
            }
            else
            {
                if (!(flags & (1 << 3)))
                    tdat = (uint8_t *)0x02300000 + (oam[sprite].num & 0xFE) * 16;
                else
                    tdat = (uint8_t *)0x02302000 + (oam[sprite].num & 0xFE) * 16;
            }

            for (int rx = 0; rx < 8; rx++)
            {
                int val, bmask;

                if (flags & (1 << 5))
                    bmask = 1 << rx;
                else
                    bmask = 1 << (7 - rx);

                val = !!(tdat[ry * 2] & bmask);
                val += !!(tdat[ry * 2 + 1] & bmask) << 1;

                if (((bx + rx) & 0xFF) < 160)
                {
                    if (val && !(flags & (1 << 7)))
                        ((uint16_t *)VRAM_1)[line * 256 + ((bx + rx) & 0xFF)] = pal[val] | 0x8000;
                    else if (val)
                    {
                        if (((uint16_t *)VRAM_1)[line * 256 + ((bx + rx) & 0xFF)] & 0x8000)
                            ((uint16_t *)VRAM_1)[line * 256 + ((bx + rx) & 0xFF)] = pal[val];
                    }
                }
            }
        }
    }

    if ((io_regs->lcdc & (1 << 0)) && has_cgb)
        draw_bg_line(abs_line, line, 1 << 7, window_active);

    if (line == 143)
    {
        // os_handle_events();

        #ifdef DISPLAY_ALL
        for (int rem = 144; rem < 256; rem++)
            draw_line(rem);
        #endif
    }
}

void increase_frameskip(void)
{
    if (!frameskip_skip)
        frameskip_skip = 1;
    else if (frameskip_draw > 1)
        frameskip_draw /= 2;
    else if (frameskip_skip < 5)
        frameskip_skip++;
    else
    {
        frameskip_skip = 0;
        frameskip_draw = 16;
    }
}

void decrease_frameskip(void)
{
    if (!frameskip_skip)
    {
        frameskip_skip = 5;
        frameskip_draw = 1;
    }
    else if (frameskip_skip > 1)
        frameskip_skip--;
    else if (frameskip_draw < 16)
        frameskip_draw *= 2;
    else
        frameskip_skip = 0;
}
