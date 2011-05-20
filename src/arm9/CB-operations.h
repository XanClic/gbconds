#define RLC(sr, cr) \
    hf(rlc_##sr) \
    { \
        r_f = (r_##sr & 0x80U) >> (7 - FS_CRY); \
        r_##sr = (r_##sr << 1) | (r_##sr >> 7); \
        if (!r_##sr) \
            r_f |= FLAG_ZERO; \
    }

#define RRC(sr, cr) \
    hf(rrc_##sr) \
    { \
        r_f = (r_##sr & 0x01) << FS_CRY; \
        r_##sr = (r_##sr >> 1) | (r_##sr << 7); \
        if (!r_##sr) \
            r_f |= FLAG_ZERO; \
    }

#define RL(sr, cr) \
    hf(rl_##sr) \
    { \
        unsigned cry = r_f & FLAG_CRY; \
        r_f = (r_##sr & 0x80U) >> (7 - FS_CRY); \
        r_##sr = (r_##sr << 1) | (cry >> FS_CRY); \
        if (!r_##sr) \
            r_f |= FLAG_ZERO; \
    }

#define RR(sr, cr) \
    hf(rr_##sr) \
    { \
        unsigned cry = r_f & FLAG_CRY; \
        r_f = (r_##sr & 0x01) << FS_CRY; \
        r_##sr = (r_##sr >> 1) | (cry << (7 - FS_CRY)); \
        if (!r_##sr) \
            r_f |= FLAG_ZERO; \
    }

#define SLA(sr, cr) \
    hf(sla_##sr) \
    { \
        r_f = (r_##sr & 0x80U) >> (7 - FS_CRY); \
        r_##sr <<= 1; \
        if (!r_##sr) \
            r_f |= FLAG_ZERO; \
    }

#define SRA(sr, cr) \
    hf(sra_##sr) \
    { \
        r_f = (r_##sr & 0x01) << FS_CRY; \
        r_##sr = (int8_t)r_##sr >> 1; \
        if (!r_##sr) \
            r_f |= FLAG_ZERO; \
    }

#define SRL(sr, cr) \
    hf(srl_##sr) \
    { \
        r_f = (r_##sr & 0x01) << FS_CRY; \
        r_##sr >>= 1; \
        if (!r_##sr) \
            r_f |= FLAG_ZERO; \
    }

#define SWAP(sr, cr) \
    hf(swap_##sr) \
    { \
        if (!r_##sr) \
            r_f = FLAG_ZERO; \
        else \
        { \
            r_##sr = (r_##sr << 4) | (r_##sr >> 4); \
            r_f = 0; \
        } \
    }

RLC(a, A)
RLC(b, B)
RLC(c, C)
RLC(d, D)
RLC(e, E)
RLC(h, H)
RLC(l, L)
RRC(a, A)
RRC(b, B)
RRC(c, C)
RRC(d, D)
RRC(e, E)
RRC(h, H)
RRC(l, L)
RL(a, A)
RL(b, B)
RL(c, C)
RL(d, D)
RL(e, E)
RL(h, H)
RL(l, L)
RR(a, A)
RR(b, B)
RR(c, C)
RR(d, D)
RR(e, E)
RR(h, H)
RR(l, L)
SLA(a, A)
SLA(b, B)
SLA(c, C)
SLA(d, D)
SLA(e, E)
SLA(h, H)
SLA(l, L)
SRA(a, A)
SRA(b, B)
SRA(c, C)
SRA(d, D)
SRA(e, E)
SRA(h, H)
SRA(l, L)
SRL(a, A)
SRL(b, B)
SRL(c, C)
SRL(d, D)
SRL(e, E)
SRL(h, H)
SRL(l, L)
SWAP(a, A)
SWAP(b, B)
SWAP(c, C)
SWAP(d, D)
SWAP(e, E)
SWAP(h, H)
SWAP(l, L)

hf(rlc__hl)
{
    uint8_t val = mem_read8(r_hl);
    r_f = (val & 0x80U) >> (7 - FS_CRY);
    val = (val << 1) | (val >> 7);
    if (!val)
        r_f |= FLAG_ZERO;
    mem_write8(r_hl, val);
}

hf(rrc__hl)
{
    uint8_t val = mem_read8(r_hl);
    r_f = (val & 0x01) << FS_CRY;
    val = (val >> 1) | (val << 7);
    if (!val)
        r_f |= FLAG_ZERO;
    mem_write8(r_hl, val);
}

hf(rl__hl)
{
    int cry = r_f & FLAG_CRY;
    uint8_t val = mem_read8(r_hl);
    r_f = (val & 0x80U) >> (7 - FS_CRY);
    val = ((val << 1) & 0xFF) | (cry >> FS_CRY);
    if (!val)
        r_f |= FLAG_ZERO;
    mem_write8(r_hl, val);
}

hf(rr__hl)
{
    int cry = r_f & FLAG_CRY;
    uint8_t val = mem_read8(r_hl);
    r_f = (val & 0x01) << FS_CRY;
    val = (val >> 1) | (cry << (7 - FS_CRY));
    if (!val)
        r_f |= FLAG_ZERO;
    mem_write8(r_hl, val);
}

hf(sla__hl)
{
    uint8_t val = mem_read8(r_hl);
    r_f = (val & 0x80U) >> (7 - FS_CRY);
    val <<= 1;
    if (!val)
        r_f |= FLAG_ZERO;
    mem_write8(r_hl, val);
}

hf(sra__hl)
{
    uint8_t val = mem_read8(r_hl);
    r_f = (val & 0x01) << FS_CRY;
    val = (int8_t)val >> 1;
    if (!val)
        r_f |= FLAG_ZERO;
    mem_write8(r_hl, val);
}

hf(srl__hl)
{
    uint8_t val = mem_read8(r_hl);
    r_f = (val & 0x01) << FS_CRY;
    val >>= 1;
    if (!val)
        r_f |= FLAG_ZERO;
    mem_write8(r_hl, val);
}

hf(swap__hl)
{
    uint8_t val = mem_read8(r_hl);
    if (!val)
        r_f = FLAG_ZERO;
    else
    {
        val = (val << 4) | (val >> 4);
        r_f = 0;
        mem_write8(r_hl, val);
    }
}

static void (*const handle0xCB[64])(void) = {
    h(0x00, rlc_b),
    h(0x01, rlc_c),
    h(0x02, rlc_d),
    h(0x03, rlc_e),
    h(0x04, rlc_h),
    h(0x05, rlc_l),
    h(0x06, rlc__hl),
    h(0x07, rlc_a),
    h(0x08, rrc_b),
    h(0x09, rrc_c),
    h(0x0A, rrc_d),
    h(0x0B, rrc_e),
    h(0x0C, rrc_h),
    h(0x0D, rrc_l),
    h(0x0E, rrc__hl),
    h(0x0F, rrc_a),
    h(0x10, rl_b),
    h(0x11, rl_c),
    h(0x12, rl_d),
    h(0x13, rl_e),
    h(0x14, rl_h),
    h(0x15, rl_l),
    h(0x16, rl__hl),
    h(0x17, rl_a),
    h(0x18, rr_b),
    h(0x19, rr_c),
    h(0x1A, rr_d),
    h(0x1B, rr_e),
    h(0x1C, rr_h),
    h(0x1D, rr_l),
    h(0x1E, rr__hl),
    h(0x1F, rr_a),
    h(0x20, sla_b),
    h(0x21, sla_c),
    h(0x22, sla_d),
    h(0x23, sla_e),
    h(0x24, sla_h),
    h(0x25, sla_l),
    h(0x26, sla__hl),
    h(0x27, sla_a),
    h(0x28, sra_b),
    h(0x29, sra_c),
    h(0x2A, sra_d),
    h(0x2B, sra_e),
    h(0x2C, sra_h),
    h(0x2D, sra_l),
    h(0x2E, sra__hl),
    h(0x2F, sra_a),
    h(0x30, swap_b),
    h(0x31, swap_c),
    h(0x32, swap_d),
    h(0x33, swap_e),
    h(0x34, swap_h),
    h(0x35, swap_l),
    h(0x36, swap__hl),
    h(0x37, swap_a),
    h(0x38, srl_b),
    h(0x39, srl_c),
    h(0x3A, srl_d),
    h(0x3B, srl_e),
    h(0x3C, srl_h),
    h(0x3D, srl_l),
    h(0x3E, srl__hl),
    h(0x3F, srl_a)
};
