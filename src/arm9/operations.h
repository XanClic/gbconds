hf(nop)
{
}

hf(prefix0x10)
{
    switch (mem_read8(r_ip))
    {
        case 0x00:
            kprintf("[%V] STOP\n", r_ip - 1);
            gbc_panic();
        default:
            kprintf("[%V] Unbekannter Opcode 0x%X (Präfix 0x10).\n", r_ip - 1, mem_read8(r_ip));
            gbc_panic();
    }
}

hf(halt)
{
    interrupt_issued = false;

    while (!interrupt_issued)
    {
        update_timer(HALT_CYCLE_STEP);
        generate_interrupts();
    }
}

hf(di)
{
    main_int_flag = false;
}

hf(ei)
{
    main_int_flag = true;
}

hf(jr)
{
    r_ip += (int8_t)mem_read8(r_ip) + 1;
}

hf(jp)
{
    r_ip = nn;
}

hf(jp__hl)
{
    r_ip = r_hl;
}

hf(call)
{
    push(r_ip + 2);
    r_ip = nn;
}

hf(ret)
{
    r_ip = pop();
}

hf(reti)
{
    main_int_flag = true;
    r_ip = pop();
}

hf(ld__nn_sp)
{
    mem_write16(nn, r_sp);
    r_ip += 2;
}

hf(ld__hl_n)
{
    mem_write8(r_hl, mem_read8(r_ip++));
}

hf(ldi__hl_a)
{
    mem_write8(r_hl++, r_a);
}

hf(ldi_a__hl)
{
    r_a = mem_read8(r_hl++);
}

hf(ldd__hl_a)
{
    mem_write8(r_hl--, r_a);
}

hf(ldd_a__hl)
{
    r_a = mem_read8(r_hl--);
}

hf(ld__nn_a)
{
    mem_write8(nn, r_a);
    r_ip += 2;
}

hf(ld_a__nn)
{
    r_a = mem_read8(nn);
    r_ip += 2;
}

hf(ld__ffn_a)
{
    mem_write8(0xFF00 + mem_read8(r_ip++), r_a);
}

hf(ld_a__ffn)
{
    r_a = mem_read8(0xFF00 + mem_read8(r_ip++));
}

hf(ld__ffc_a)
{
    mem_write8(0xFF00 + r_c, r_a);
}

hf(ld_a__ffc)
{
    r_a = mem_read8(0xFF00 + r_c);
}

hf(ld_sp_hl)
{
    r_sp = r_hl;
}

hf(ld_hl_sps)
{
    int val = (int8_t)mem_read8(r_ip++);
    unsigned result = (unsigned)(r_sp + val);

    r_f = ((result & 0x10000) >> (16 - FS_CRY)) | (((r_sp ^ (unsigned)val ^ result) & 0x1000) >> (12 - FS_HCRY));
    r_hl = result;
}

hf(add_sp_s)
{
    int val = (int8_t)mem_read8(r_ip++);
    unsigned result = (unsigned)(r_sp + val);

    r_f = ((result & 0x10000) >> (16 - FS_CRY)) | (((r_sp ^ (unsigned)val ^ result) & 0x1000) >> (12 - FS_HCRY));
    r_sp = result;
}

hf(add_hl_hl)
{
    r_f = (r_f & FLAG_ZERO) | ((r_hl & 0x8000) >> (15 - FS_CRY)) | ((r_hl & 0x800) >> (11 - FS_CRY));
    r_hl <<= 1;
}

hf(rlca)
{
    r_f = (r_a & 0x80U) >> (7 - FS_CRY);
    r_a = (r_a << 1) | (r_a >> 7);
}

hf(rla)
{
    unsigned cry = r_f & FLAG_CRY;
    r_f = (r_a & 0x80U) >> (7 - FS_CRY);
    r_a = (r_a << 1) | (cry >> FS_CRY);
}

hf(rrca)
{
    r_f = (r_a & 0x01) << FS_CRY;
    r_a = (r_a >> 1) | (r_a << 7);
}

hf(rra)
{
    unsigned cry = r_f & FLAG_CRY;
    r_f = (r_a & 0x01) << FS_CRY;
    r_a = (r_a >> 1) | (cry << (7 - FS_CRY));
}

hf(cpl_a)
{
    r_a ^= 0xFF;
    r_f |= FLAG_SUB | FLAG_HCRY;
}

hf(daa)
{
    r_af = daa_table[r_a | ((r_f & 0x70) << 4)];
}

hf(scf)
{
    r_f = (r_f & FLAG_ZERO) | FLAG_CRY;
}

hf(ccf)
{
    r_f = (r_f & FLAG_ZERO) | ((r_f & FLAG_CRY) ^ FLAG_CRY);
}

/// LD ??, nn

LD_xx_nn(bc)
LD_xx_nn(de)
LD_xx_nn(hl)
LD_xx_nn(sp)

/// LD (??), A

LD__xx_A(bc)
LD__xx_A(de)
LD__xx_A(hl)

/// INC ??

INC16(bc)
INC16(de)
INC16(hl)
INC16(sp)

/// DEC ??

DEC16(bc)
DEC16(de)
DEC16(hl)
DEC16(sp)

/// INC ?

INC(a)
INC(b)
INC(c)
INC(d)
INC(e)
INC(h)
INC(l)

hf(inc__hl)
{
    uint8_t val = mem_read8(r_hl);
    r_f = (r_f & FLAG_CRY) | (!++val << FS_ZERO);
    r_f |= !(val & 0xF) << FS_HCRY;
    mem_write8(r_hl, val);
}

/// DEC ?

DEC(a)
DEC(b)
DEC(c)
DEC(d)
DEC(e)
DEC(h)
DEC(l)

hf(dec__hl)
{
    uint8_t val = mem_read8(r_hl);
    r_f = FLAG_SUB | (r_f & FLAG_CRY) | (!(val & 0xF) << FS_HCRY);
    r_f |= !--val << FS_ZERO;
    mem_write8(r_hl, val);
}

/// LD ?, n

LD_x_n(a)
LD_x_n(b)
LD_x_n(c)
LD_x_n(d)
LD_x_n(e)
LD_x_n(h)
LD_x_n(l)

/// ADD HL, ??

ADD_HL(bc)
ADD_HL(de)
ADD_HL(sp)

/// LD A, (??)

LD_A__(bc)
LD_A__(de)
LD_A__(hl)

/// JRcc n

JRcc( z,   r_f & FLAG_ZERO )
JRcc(nz, !(r_f & FLAG_ZERO))
JRcc( c,   r_f & FLAG_CRY  )
JRcc(nc, !(r_f & FLAG_CRY ))

/// JPcc nn

JPcc( z,   r_f & FLAG_ZERO )
JPcc(nz, !(r_f & FLAG_ZERO))
JPcc( c,   r_f & FLAG_CRY  )
JPcc(nc, !(r_f & FLAG_CRY ))


/// CALLcc nn

CALLcc( z,   r_f & FLAG_ZERO )
CALLcc(nz, !(r_f & FLAG_ZERO))
CALLcc( c,   r_f & FLAG_CRY  )
CALLcc(nc, !(r_f & FLAG_CRY ))


/// RETcc

RETcc( z,   r_f & FLAG_ZERO )
RETcc(nz, !(r_f & FLAG_ZERO))
RETcc( c,   r_f & FLAG_CRY  )
RETcc(nc, !(r_f & FLAG_CRY ))

/// LD ?, ?

hf(ld_a_a) {}
LD_x_x(a, b)
LD_x_x(a, c)
LD_x_x(a, d)
LD_x_x(a, e)
LD_x_x(a, h)
LD_x_x(a, l)
// "LD A, (HL)" wurde bereits als "LD A, (??)" definiert

LD_x_x(b, a)
hf(ld_b_b) {}
LD_x_x(b, c)
LD_x_x(b, d)
LD_x_x(b, e)
LD_x_x(b, h)
LD_x_x(b, l)
LD_x_x_(b, mem_read8(r_hl), _hl)

LD_x_x(c, a)
LD_x_x(c, b)
hf(ld_c_c) {}
LD_x_x(c, d)
LD_x_x(c, e)
LD_x_x(c, h)
LD_x_x(c, l)
LD_x_x_(c, mem_read8(r_hl), _hl)

LD_x_x(d, a)
LD_x_x(d, b)
LD_x_x(d, c)
hf(ld_d_d) {}
LD_x_x(d, e)
LD_x_x(d, h)
LD_x_x(d, l)
LD_x_x_(d, mem_read8(r_hl), _hl)

LD_x_x(e, a)
LD_x_x(e, b)
LD_x_x(e, c)
LD_x_x(e, d)
hf(ld_e_e) {}
LD_x_x(e, h)
LD_x_x(e, l)
LD_x_x_(e, mem_read8(r_hl), _hl)

LD_x_x(h, a)
LD_x_x(h, b)
LD_x_x(h, c)
LD_x_x(h, d)
LD_x_x(h, e)
hf(ld_h_h) {}
LD_x_x(h, l)
LD_x_x_(h, mem_read8(r_hl), _hl)

LD_x_x(l, a)
LD_x_x(l, b)
LD_x_x(l, c)
LD_x_x(l, d)
LD_x_x(l, e)
LD_x_x(l, h)
hf(ld_l_l) {}
LD_x_x_(l, mem_read8(r_hl), _hl)

/// LD (HL), ?

// "LD (HL), A" wurde bereits als "LD (??), A" definiert
LD__HL_x(b)
LD__HL_x(c)
LD__HL_x(d)
LD__HL_x(e)
LD__HL_x(h)
LD__HL_x(l)

/// ADD A, ?

ADD_A(b)
ADD_A(c)
ADD_A(d)
ADD_A(e)
ADD_A(h)
ADD_A(l)
ADD_A_(mem_read8(r_hl), _hl)
ADD_A_(mem_read8(r_ip++), n)

hf(add_a_a)
{
    if (!r_a)
        r_f = FLAG_ZERO;
    else
    {
        if (r_a & 0x80)
            r_f = FLAG_CRY;
        else
            r_f = 0;

        if (r_a & 0x08)
            r_f |= FLAG_HCRY;

        r_a <<= 1;

        if (!r_a)
            r_f |= FLAG_ZERO;
    }
}

/// ADC A, ?

ADC_A(a)
ADC_A(b)
ADC_A(c)
ADC_A(d)
ADC_A(e)
ADC_A(h)
ADC_A(l)
ADC_A_(mem_read8(r_hl), _hl)
ADC_A_(mem_read8(r_ip++), n)

/// SUB A, ?

SUB_A(b)
SUB_A(c)
SUB_A(d)
SUB_A(e)
SUB_A(h)
SUB_A(l)
SUB_A_(mem_read8(r_hl), _hl)
SUB_A_(mem_read8(r_ip++), n)

hf(sub_a_a)
{
    r_a = 0;
    r_f = FLAG_SUB | FLAG_ZERO;
}

/// SBC A, ?

SBC_A(b)
SBC_A(c)
SBC_A(d)
SBC_A(e)
SBC_A(h)
SBC_A(l)
SBC_A_(mem_read8(r_hl), _hl)
SBC_A_(mem_read8(r_ip++), n)

hf(sbc_a_a)
{
    if (r_f & FLAG_CRY)
    {
        r_a = 0xFF;
        r_f = FLAG_SUB | FLAG_ZERO | FLAG_CRY | FLAG_HCRY;
    }
    else
    {
        r_a = 0;
        r_f = FLAG_SUB | FLAG_ZERO;
    }
}


/// AND A, ?

AND_A(b)
AND_A(c)
AND_A(d)
AND_A(e)
AND_A(h)
AND_A(l)
AND_A_(mem_read8(r_hl), _hl)
AND_A_(mem_read8(r_ip++), n)

hf(and_a_a)
{
    r_f = FLAG_HCRY | (!r_a << FS_ZERO);
}

/// XOR A, ?

XOR_A(b)
XOR_A(c)
XOR_A(d)
XOR_A(e)
XOR_A(h)
XOR_A(l)
XOR_A_(mem_read8(r_hl), _hl)
XOR_A_(mem_read8(r_ip++), n)

hf(xor_a_a)
{
    r_a = 0;
    r_f = FLAG_ZERO;
}

/// OR A, ?

OR_A(b)
OR_A(c)
OR_A(d)
OR_A(e)
OR_A(h)
OR_A(l)
OR_A_(mem_read8(r_hl), _hl)
OR_A_(mem_read8(r_ip++), n)

hf(or_a_a)
{
    r_f = !r_a << FS_ZERO;
}

/// CP a, r/n

CP_A(b)
CP_A(c)
CP_A(d)
CP_A(e)
CP_A(h)
CP_A(l)
CP_A_(mem_read8(r_hl), _hl)
CP_A_(mem_read8(r_ip++), n)

hf(cp_a_a)
{
    r_f = FLAG_SUB | FLAG_ZERO;
}

/// PUSH ??

PUSH(af)
PUSH(bc)
PUSH(de)
PUSH(hl)


/// POP ??

POP(af)
POP(bc)
POP(de)
POP(hl)


/// RST

RST(0x00)
RST(0x08)
RST(0x10)
RST(0x18)
RST(0x20)
RST(0x28)
RST(0x30)
RST(0x38)

#include "CB-operations.h"

#define BIT_CHECK(regnum, regname) \
            case regnum: \
            r_f = (r_f & FLAG_CRY) | FLAG_HCRY | ((r_##regname & bval) ? 0 : FLAG_ZERO); \
            break;

#define BIT_RESET(regnum, regname) \
            case regnum: \
            r_##regname &= ~bval; \
            break;

#define BIT_SET(regnum, regname) \
            case regnum: \
            r_##regname |= bval; \
            break;

hf(prefix0xCB)
{
    int bval = 1 << ((mem_read8(r_ip) & 0x38) >> 3);
    int reg = mem_read8(r_ip) & 0x07;

    switch ((mem_read8(r_ip++) & 0xC0) >> 6)
    {
        case 0x01: // BIT
            switch (reg)
            {
                BIT_CHECK(7, a)
                BIT_CHECK(0, b)
                BIT_CHECK(1, c)
                BIT_CHECK(2, d)
                BIT_CHECK(3, e)
                BIT_CHECK(4, h)
                BIT_CHECK(5, l)
                case 6:
                    r_f = (r_f & FLAG_CRY) | FLAG_HCRY | ((mem_read8(r_hl) & bval) ? 0 : FLAG_ZERO); \
                    break;
            }
            break;
        case 0x02: // RES
            switch (reg)
            {
                BIT_RESET(7, a)
                BIT_RESET(0, b)
                BIT_RESET(1, c)
                BIT_RESET(2, d)
                BIT_RESET(3, e)
                BIT_RESET(4, h)
                BIT_RESET(5, l)
                case 6:
                    mem_write8(r_hl, mem_read8(r_hl) & ~bval);
            }
            break;
        case 0x03: // SET
            switch (reg)
            {
                BIT_SET(7, a)
                BIT_SET(0, b)
                BIT_SET(1, c)
                BIT_SET(2, d)
                BIT_SET(3, e)
                BIT_SET(4, h)
                BIT_SET(5, l)
                case 6:
                    mem_write8(r_hl, mem_read8(r_hl) | bval);
            }
            break;
        default:
            handle0xCB[mem_read8(r_ip - 1)]();
    }
}

static void (*const handle[256])(void) = {
    h(0x00, nop),
    h(0x01, ld_bc_nn),
    h(0x02, ld__bc_a),
    h(0x03, inc_bc),
    h(0x04, inc_b),
    h(0x05, dec_b),
    h(0x06, ld_b_n),
    h(0x07, rlca),
    h(0x08, ld__nn_sp),
    h(0x09, add_hl_bc),
    h(0x0A, ld_a__bc),
    h(0x0B, dec_bc),
    h(0x0C, inc_c),
    h(0x0D, dec_c),
    h(0x0E, ld_c_n),
    h(0x0F, rrca),
    h(0x10, prefix0x10),
    h(0x11, ld_de_nn),
    h(0x12, ld__de_a),
    h(0x13, inc_de),
    h(0x14, inc_d),
    h(0x15, dec_d),
    h(0x16, ld_d_n),
    h(0x17, rla),
    h(0x18, jr),
    h(0x19, add_hl_de),
    h(0x1A, ld_a__de),
    h(0x1B, dec_de),
    h(0x1C, inc_e),
    h(0x1D, dec_e),
    h(0x1E, ld_e_n),
    h(0x1F, rra),
    h(0x20, jrnz),
    h(0x21, ld_hl_nn),
    h(0x22, ldi__hl_a),
    h(0x23, inc_hl),
    h(0x24, inc_h),
    h(0x25, dec_h),
    h(0x26, ld_h_n),
    h(0x27, daa),
    h(0x28, jrz),
    h(0x29, add_hl_hl),
    h(0x2A, ldi_a__hl),
    h(0x2B, dec_hl),
    h(0x2C, inc_l),
    h(0x2D, dec_l),
    h(0x2E, ld_l_n),
    h(0x2F, cpl_a),
    h(0x30, jrnc),
    h(0x31, ld_sp_nn),
    h(0x32, ldd__hl_a),
    h(0x33, inc_sp),
    h(0x34, inc__hl),
    h(0x35, dec__hl),
    h(0x36, ld__hl_n),
    h(0x37, scf),
    h(0x38, jrc),
    h(0x39, add_hl_sp),
    h(0x3A, ldd_a__hl),
    h(0x3B, dec_sp),
    h(0x3C, inc_a),
    h(0x3D, dec_a),
    h(0x3E, ld_a_n),
    h(0x3F, ccf),
    h(0x40, ld_b_b),
    h(0x41, ld_b_c),
    h(0x42, ld_b_d),
    h(0x43, ld_b_e),
    h(0x44, ld_b_h),
    h(0x45, ld_b_l),
    h(0x46, ld_b__hl),
    h(0x47, ld_b_a),
    h(0x48, ld_c_b),
    h(0x49, ld_c_c),
    h(0x4A, ld_c_d),
    h(0x4B, ld_c_e),
    h(0x4C, ld_c_h),
    h(0x4D, ld_c_l),
    h(0x4E, ld_c__hl),
    h(0x4F, ld_c_a),
    h(0x50, ld_d_b),
    h(0x51, ld_d_c),
    h(0x52, ld_d_d),
    h(0x53, ld_d_e),
    h(0x54, ld_d_h),
    h(0x55, ld_d_l),
    h(0x56, ld_d__hl),
    h(0x57, ld_d_a),
    h(0x58, ld_e_b),
    h(0x59, ld_e_c),
    h(0x5A, ld_e_d),
    h(0x5B, ld_e_e),
    h(0x5C, ld_e_h),
    h(0x5D, ld_e_l),
    h(0x5E, ld_e__hl),
    h(0x5F, ld_e_a),
    h(0x60, ld_h_b),
    h(0x61, ld_h_c),
    h(0x62, ld_h_d),
    h(0x63, ld_h_e),
    h(0x64, ld_h_h),
    h(0x65, ld_h_l),
    h(0x66, ld_h__hl),
    h(0x67, ld_h_a),
    h(0x68, ld_l_b),
    h(0x69, ld_l_c),
    h(0x6A, ld_l_d),
    h(0x6B, ld_l_e),
    h(0x6C, ld_l_h),
    h(0x6D, ld_l_l),
    h(0x6E, ld_l__hl),
    h(0x6F, ld_l_a),
    h(0x70, ld__hl_b),
    h(0x71, ld__hl_c),
    h(0x72, ld__hl_d),
    h(0x73, ld__hl_e),
    h(0x74, ld__hl_h),
    h(0x75, ld__hl_l),
    h(0x76, halt),
    h(0x77, ld__hl_a),
    h(0x78, ld_a_b),
    h(0x79, ld_a_c),
    h(0x7A, ld_a_d),
    h(0x7B, ld_a_e),
    h(0x7C, ld_a_h),
    h(0x7D, ld_a_l),
    h(0x7E, ld_a__hl),
    h(0x7F, ld_a_a),
    h(0x80, add_a_b),
    h(0x81, add_a_c),
    h(0x82, add_a_d),
    h(0x83, add_a_e),
    h(0x84, add_a_h),
    h(0x85, add_a_l),
    h(0x86, add_a__hl),
    h(0x87, add_a_a),
    h(0x88, adc_a_b),
    h(0x89, adc_a_c),
    h(0x8A, adc_a_d),
    h(0x8B, adc_a_e),
    h(0x8C, adc_a_h),
    h(0x8D, adc_a_l),
    h(0x8E, adc_a__hl),
    h(0x8F, adc_a_a),
    h(0x90, sub_a_b),
    h(0x91, sub_a_c),
    h(0x92, sub_a_d),
    h(0x93, sub_a_e),
    h(0x94, sub_a_h),
    h(0x95, sub_a_l),
    h(0x96, sub_a__hl),
    h(0x97, sub_a_a),
    h(0x98, sbc_a_b),
    h(0x99, sbc_a_c),
    h(0x9A, sbc_a_d),
    h(0x9B, sbc_a_e),
    h(0x9C, sbc_a_h),
    h(0x9D, sbc_a_l),
    h(0x9E, sbc_a__hl),
    h(0x9F, sbc_a_a),
    h(0xA0, and_a_b),
    h(0xA1, and_a_c),
    h(0xA2, and_a_d),
    h(0xA3, and_a_e),
    h(0xA4, and_a_h),
    h(0xA5, and_a_l),
    h(0xA6, and_a__hl),
    h(0xA7, and_a_a),
    h(0xA8, xor_a_b),
    h(0xA9, xor_a_c),
    h(0xAA, xor_a_d),
    h(0xAB, xor_a_e),
    h(0xAC, xor_a_h),
    h(0xAD, xor_a_l),
    h(0xAE, xor_a__hl),
    h(0xAF, xor_a_a),
    h(0xB0, or_a_b),
    h(0xB1, or_a_c),
    h(0xB2, or_a_d),
    h(0xB3, or_a_e),
    h(0xB4, or_a_h),
    h(0xB5, or_a_l),
    h(0xB6, or_a__hl),
    h(0xB7, or_a_a),
    h(0xB8, cp_a_b),
    h(0xB9, cp_a_c),
    h(0xBA, cp_a_d),
    h(0xBB, cp_a_e),
    h(0xBC, cp_a_h),
    h(0xBD, cp_a_l),
    h(0xBE, cp_a__hl),
    h(0xBF, cp_a_a),
    h(0xC0, retnz),
    h(0xC1, pop_bc),
    h(0xC2, jpnz),
    h(0xC3, jp),
    h(0xC4, callnz),
    h(0xC5, push_bc),
    h(0xC6, add_a_n),
    h(0xC7, rst0x00),
    h(0xC8, retz),
    h(0xC9, ret),
    h(0xCA, jpz),
    h(0xCB, prefix0xCB),
    h(0xCC, callz),
    h(0xCD, call),
    h(0xCE, adc_a_n),
    h(0xCF, rst0x08),
    h(0xD0, retnc),
    h(0xD1, pop_de),
    h(0xD2, jpnc),
    h(0xD4, callnc),
    h(0xD5, push_de),
    h(0xD6, sub_a_n),
    h(0xD7, rst0x10),
    h(0xD8, retc),
    h(0xD9, reti),
    h(0xDA, jpc),
    h(0xDC, callc),
    h(0xDE, sbc_a_n),
    h(0xDF, rst0x18),
    h(0xE0, ld__ffn_a),
    h(0xE1, pop_hl),
    h(0xE2, ld__ffc_a),
    h(0xE5, push_hl),
    h(0xE6, and_a_n),
    h(0xE7, rst0x20),
    h(0xE8, add_sp_s),
    h(0xE9, jp__hl),
    h(0xEA, ld__nn_a),
    h(0xEE, xor_a_n),
    h(0xEF, rst0x28),
    h(0xF0, ld_a__ffn),
    h(0xF1, pop_af),
    h(0xF2, ld_a__ffc),
    h(0xF3, di),
    h(0xF5, push_af),
    h(0xF6, or_a_n),
    h(0xF7, rst0x30),
    h(0xF8, ld_hl_sps),
    h(0xF9, ld_sp_hl),
    h(0xFA, ld_a__nn),
    h(0xFB, ei),
    h(0xFE, cp_a_n),
    h(0xFF, rst0x38)
};
