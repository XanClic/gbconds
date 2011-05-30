#ifndef GEN_OPERATIONS_H
#define GEN_OPERATIONS_H

/// LD ?, n

#define LD_x_n(reg) \
hf(ld_##reg##_n) \
{ \
    r_##reg = mem_read8(r_ip++); \
}


/// LD ??, nn

#define LD_xx_nn(reg) \
hf(ld_##reg##_nn) \
{ \
    r_##reg = nn; \
    r_ip += 2; \
}


/// LD (??), A

#define LD__xx_A(reg) \
hf(ld__##reg##_a) \
{ \
    mem_write8(r_##reg, r_a); \
}


/// LD A, (??)

#define LD_A__(reg) \
hf(ld_a__##reg) \
{ \
    r_a = mem_read8(r_##reg); \
}


/// LD ?, ?

#define LD_x_x_(r1, r2, r2a) \
hf(ld_##r1##_##r2a) \
{ \
    r_##r1 = r2; \
}

#define LD_x_x(r1, r2) LD_x_x_(r1, r_##r2, r2)


/// LD (HL), ?

#define LD__HL_x(reg) \
hf(ld__hl_##reg) \
{ \
    mem_write8(r_hl, r_##reg); \
}


/// INC ?

#define INC(reg) \
hf(inc_##reg) \
{ \
    r_f = (r_f & FLAG_CRY) | (!++r_##reg << FS_ZERO); \
    r_f |= !(r_##reg & 0xF) << FS_HCRY; \
}


/// INC ??

#define INC16(reg) \
hf(inc_##reg) \
{ \
    r_##reg++; \
}


/// DEC ?

#define DEC(reg) \
hf(dec_##reg) \
{ \
    r_f = FLAG_SUB | (r_f & FLAG_CRY) | (!(r_##reg & 0xF) << FS_HCRY); \
    r_f |= !--r_##reg << FS_ZERO; \
}


/// DEC ??

#define DEC16(reg) \
hf(dec_##reg) \
{ \
    r_##reg--; \
}


/// ADD HL, ??

#define ADD_HL(reg) \
hf(add_hl_##reg) \
{ \
    unsigned result = r_hl + r_##reg; \
    r_f = (r_f & FLAG_ZERO) | ((result & 0x10000U) >> (16 - FS_CRY)) | (((r_hl ^ r_##reg ^ result) & 0x1000U) >> (12 - FS_HCRY)); \
    r_hl = (uint16_t)result; \
}


/// ADD A, ?

#define ADD_A_(reg, ar) \
hf(add_a_##ar) \
{ \
    uint8_t rval = reg; \
    uint16_t result = (uint16_t)(r_a + rval); \
    r_f = ((result & 0x100U) >> (8 - FS_CRY)) | (!(result & 0xFF) << FS_ZERO); \
    if ((r_a & 0xF) + (rval & 0xF) > 0xF) \
        r_f |= FLAG_HCRY; \
    r_a = (uint8_t)result; \
}

#define ADD_A(reg) ADD_A_(r_##reg, reg)


/// ADC A, ?

#define ADC_A_(reg, ar) \
hf(adc_a_##ar) \
{ \
    uint8_t rval = reg; \
    int cry = (r_f & FLAG_CRY) >> FS_CRY; \
    uint16_t result = (uint16_t)(r_a + rval + cry); \
    r_f = ((result & 0x100U) >> (8 - FS_CRY)) | (!(result & 0xFF) << FS_ZERO); \
    if ((r_a & 0xF) + (rval & 0xF) + cry > 0xF) \
        r_f |= FLAG_HCRY; \
    r_a = (uint8_t)result; \
}

#define ADC_A(reg) ADC_A_(r_##reg, reg)


/// SUB A, ?

#define SUB_A_(reg, ar) \
hf(sub_a_##ar) \
{ \
    uint8_t rval = reg; \
    uint16_t result = (uint16_t)(r_a - rval); \
    r_f = FLAG_SUB | ((result & 0x100U) >> (8 - FS_CRY)) | (!(result & 0xFF) << FS_ZERO); \
    if (((r_a & 0xFU) - (rval & 0xFU)) & 0x10U) \
        r_f |= FLAG_HCRY; \
    r_a = (uint8_t)result; \
}

#define SUB_A(reg) SUB_A_(r_##reg, reg)


/// SBC A, ?

#define SBC_A_(reg, ar) \
hf(sbc_a_##ar) \
{ \
    uint8_t rval = reg; \
    int cry = (r_f & FLAG_CRY) >> FS_CRY; \
    uint16_t result = (uint16_t)(r_a - rval - cry); \
    r_f = FLAG_SUB | ((result & 0x100U) >> (8 - FS_CRY)) | (!(result & 0xFF) << FS_ZERO); \
    if (((r_a & 0xFU) - (rval & 0xFU) - (unsigned)cry) & 0x10U) \
        r_f |= FLAG_HCRY; \
    r_a = (uint8_t)result; \
}

#define SBC_A(reg) SBC_A_(r_##reg, reg)


/// CP A, ?

#define CP_A_(reg, ar) \
hf(cp_a_##ar) \
{ \
    uint8_t rval = reg; \
    uint16_t result = (uint16_t)(r_a - rval); \
    r_f = FLAG_SUB | ((result & 0x100U) >> (8 - FS_CRY)) | (!(result & 0xFF) << FS_ZERO); \
    if (((r_a & 0xFU) - (rval & 0xFU)) & 0x10U) \
        r_f |= FLAG_HCRY; \
}

#define CP_A(reg) CP_A_(r_##reg, reg)


/// AND A, ?

#define AND_A_(reg, ar) \
hf(and_a_##ar) \
{ \
    r_a &= reg; \
    r_f = FLAG_HCRY | (!r_a << FS_ZERO); \
}

#define AND_A(reg) AND_A_(r_##reg, reg)


/// XOR A, ?

#define XOR_A_(reg, ar) \
hf(xor_a_##ar) \
{ \
    r_a ^= reg; \
    r_f = !r_a << FS_ZERO; \
}

#define XOR_A(reg) XOR_A_(r_##reg, reg)


/// OR A, ?

#define OR_A_(reg, ar) \
hf(or_a_##ar) \
{ \
    r_a |= reg; \
    r_f = !r_a << FS_ZERO; \
}

#define OR_A(reg) OR_A_(r_##reg, reg)


/// JRcc n

#define JRcc(cc, condition) \
hf(jr##cc) \
{ \
    if (condition) \
        __op_handler_jr(); \
    else \
        r_ip++; \
}


/// JPcc nn

#define JPcc(cc, condition) \
hf(jp##cc) \
{ \
    if (condition) \
        __op_handler_jp(); \
    else \
        r_ip += 2; \
}


/// CALLcc nn

#define CALLcc(cc, condition) \
hf(call##cc) \
{ \
    if (condition) \
        __op_handler_call(); \
    else \
        r_ip += 2; \
}


/// RETcc

#define RETcc(cc, condition) \
hf(ret##cc) \
{ \
    if (condition) \
        __op_handler_ret(); \
}


/// PUSH ??

#define PUSH(reg) \
hf(push_##reg) \
{ \
    push(r_##reg); \
}


/// POP ??

#define POP(reg) \
hf(pop_##reg) \
{ \
    r_##reg = pop(); \
}


/// RST

#define RST(addr) \
hf(rst##addr) \
{ \
    push(r_ip); \
    r_ip = addr; \
}

#endif
