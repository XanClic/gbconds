#include <config.h>
#include <io.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <video.h>

#define DISP (VRAM_1 + 144 * 256 * 2)
#define DISP_W 256
#define DISP_H 48

extern const void _binary_font_npf_start, _binary_font_npf_end;

static const struct npf
{
    char sig[3], version;
    uint16_t height, width;
    char name[24];
} __attribute__((packed)) *font;

static const struct npf_char
{
    uint32_t num;
    uint8_t rows[];
} __attribute__((packed)) *chars;

static uint16_t ascii_chars[128] = { 0xFFFF }, inv_char = 0xFFFF;

static unsigned fw, fh, tw, th;
static size_t charsz;

void init_video(void)
{
    font = &_binary_font_npf_start;

    vw32(IO, 0x0304, 0x8203); // LCDs und 2D-Engines ein, Bildschirm A nach oben

    vw8(IO, 0x0240, 0x80); // VRAM A initialisieren

    // Oberen Bildschirme in den „rohen Modus“ schalten
    vw32(IO, 0x0000, (2 << 16) | (0 << 18)); // VRAM A

    if (strncmp(font->sig, "NPF", 3) || (font->version != '2'))
        for (;;);

    fw = font->width;
    fh = font->height;

    // Fonts mit mehr als einem Byte pro Zeile unterstützt dieser Treiber nicht
    if (fw > 8)
        for (;;);

    tw = DISP_W / fw;
    th = DISP_H / fh;

    charsz = ((fw + 7) / 8) * fh + sizeof(uint32_t);

    chars = (const struct npf_char *)(font + 1);

    const struct npf_char *chr = chars;
    unsigned chari = 0;
    while ((uintptr_t)chr < (uintptr_t)&_binary_font_npf_end)
    {
        if (chr->num == 0xFFFD)
            inv_char = chari;
        else if (chr->num < 128)
            ascii_chars[chr->num] = chari;

        chr = (const struct npf_char *)((uintptr_t)chr + charsz);
        chari++;
    }

    if (inv_char == 0xFFFF)
        inv_char = ascii_chars[' '];

    for (size_t c = 0; c < 256; c++)
        if (ascii_chars[c] == 0xFFFF)
            ascii_chars[c] = inv_char;

    cls(rgb24to15(0, 0, 0));
}

void cls(rgb_t color)
{
    memsetptr((void *)DISP, color | (color << 16), DISP_W * DISP_H / 2);
}

void term_putc(unsigned x, unsigned y, unsigned unicode)
{
    const struct npf_char *chr = chars;

    if (unicode < 128)
        chr = (const struct npf_char *)((uintptr_t)chars + charsz * ascii_chars[unicode]);
    else
    {
        while ((uintptr_t)chr < (uintptr_t)&_binary_font_npf_end)
        {
            if (chr->num == unicode)
                break;
            chr = (const struct npf_char *)((uintptr_t)chr + charsz);
        }

        if ((uintptr_t)chr >= (uintptr_t)&_binary_font_npf_end)
            chr = (const struct npf_char *)((uintptr_t)chars + charsz * inv_char);
    }

    uint16_t *out = (uint16_t *)DISP + (y * fh * 256 + x * fw);

    for (unsigned ry = 0; ry < fh; ry++)
    {
        uint8_t row = chr->rows[ry];

        for (unsigned rx = 0; rx < fw; rx++)
            out[rx] = (row & (1 << rx)) ? rgb24to15(170, 170, 170) : rgb24to15(0, 0, 0);

        out += 256;
    }
}

static void scroll_up(void)
{
    uint32_t *src = (uint32_t *)(DISP + 256 * fh * 2);
    uint32_t *dst = (uint32_t *)DISP;
    uint32_t count = 256 * (th - 1) * fh / 2;
    while (count--)
        *(dst++) = *(src++);
    count = 256 * fh / 2;
    while (count--)
        *(dst++) = rgb24to15(0, 0, 0) | (rgb24to15(0, 0, 0) << 16);
}

void putc(int c)
{
    static unsigned x = 0, y = 0, cur_uni = 0, uni_remaining = 0;

    if (uni_remaining)
    {
        if ((c & 0xC0) == 0x80)
        {
            cur_uni = (cur_uni << 6) | (c & 0x3F);
            if (--uni_remaining)
                return;
            c = cur_uni;
        }
        else
        {
            uni_remaining = 0;
            c = 0xFFFD;
        }
    }

    if ((c & 0xF8) == 0xF0)
    {
        cur_uni = c & 0x07;
        uni_remaining = 3;
        return;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        cur_uni = c & 0x0F;
        uni_remaining = 2;
        return;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        cur_uni = c & 0x1F;
        uni_remaining = 1;
        return;
    }

    switch (c)
    {
        case '\n':
            x = 0;
            if (++y >= th)
            {
                y--;
                scroll_up();
            }
        case '\r':
            x = 0;
            break;
        default:
            term_putc(x++, y, c);
            if (x >= tw)
            {
                x = 0;
                if (++y >= th)
                {
                    y--;
                    scroll_up();
                }
            }
    }
}

void kputs(const char *s)
{
    while (*s)
        putc(*(s++));
}

void kprintf(const char *s, ...)
{
    va_list va;

    va_start(va, s);

    while (*s)
    {
        switch (*(s++))
        {
            case '%':
                switch (*(s++))
                {
                    case '%':
                        putc('%');
                        break;
                    case 's':
                    {
                        const char *ns = va_arg(va, const char *);
                        while (*ns)
                            putc(*(ns++));
                        break;
                    }
                    case 'c':
                        putc(va_arg(va, int));
                        break;
                    case 'd':
                    case 'i':
                    {
                        int num = va_arg(va, int);
                        if (!num)
                            putc('0');
                        else
                        {
                            if (num < 0)
                            {
                                putc('-');
                                num *= -1;
                            }

                            char buf[11];
                            int i = 11;
                            while (num)
                            {
                                buf[--i] = num % 10 + '0';
                                num /= 10;
                            }
                            while (i < 11)
                                putc(buf[i++]);
                        }
                        break;
                    }
                    case 'x':
                    {
                        unsigned num = va_arg(va, unsigned);
                        if (!num)
                            putc('0');
                        else
                        {
                            int i;
                            for (i = 0; !(num >> 28); i++)
                                num <<= 4;
                            for (; i < 8; i++)
                            {
                                putc((num >> 28) + ((num >> 28) < 10 ? '0' : 'a' - 10));
                                num <<= 4;
                            }
                        }
                        break;
                    }
                    case 'X':
                    {
                        unsigned num = va_arg(va, unsigned);
                        if (!num)
                            putc('0');
                        else
                        {
                            int i;
                            for (i = 0; !(num >> 28); i++)
                                num <<= 4;
                            for (; i < 8; i++)
                            {
                                putc((num >> 28) + ((num >> 28) < 10 ? '0' : 'A' - 10));
                                num <<= 4;
                            }
                        }
                        break;
                    }
                    case 'p':
                    {
                        unsigned num = va_arg(va, unsigned);
                        putc('0');
                        putc('x');
                        for (int i = 0; i < 8; i++)
                        {
                            putc((num >> 28) + ((num >> 28) < 10 ? '0' : 'a' - 10));
                            num <<= 4;
                        }
                        break;
                    }
                    case 'V':
                    {
                        uint16_t num = va_arg(va, unsigned);
                        for (int i = 0; i < 4; i++)
                        {
                            putc((num >> 12) + ((num >> 12) < 10 ? '0' : 'A' - 10));
                            num <<= 4;
                        }
                        break;
                    }
                    case 'v':
                    {
                        uint16_t num = va_arg(va, unsigned);
                        for (int i = 0; i < 4; i++)
                        {
                            putc((num >> 12) + ((num >> 12) < 10 ? '0' : 'a' - 10));
                            num <<= 4;
                        }
                        break;
                    }
                    default:
                        putc('%');
                        putc(s[-1]);
                }
                break;
            default:
                putc(s[-1]);
        }
    }

    va_end(va);
}
