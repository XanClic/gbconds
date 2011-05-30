.arm
.global execute
.extern init_env
.extern mem_read8
.extern mem_read16
.extern kprintf
.extern gbc_panic
.extern update_timer

.extern has_cgb
.extern main_int_flag
.extern cycles
.extern cycles0xCB
.extern fr_af
.extern fr_bc
.extern fr_de
.extern fr_hl
.extern fr_ip
.extern fr_sp

.equ CYCLE_STEP,16
.equ HALT_CYCLE_STEP,32

#define true  1
#define false 0

/* Registerverteilung:
 *  r4: AF
 *  r5: BC
 *  r6: DE
 *  r7: HL
 *  r8: IP
 *  r9: SP
 * r10: cycles_gone
 */

storage:
.4byte 0

execute:
stmdb   r13!,{r4-r11,r14}

ldr     r0,[r15,#has_cgb_addr-8-.]
ldr     r0,[r0]
cmp     r0,#0
mov     r4,#0x01B0
orrne   r4,#0x1000
moveq   r5,#0x0013
movne   r5,#0x0000
moveq   r6,#0x00D8
movne   r6,#0xFF00
orrne   r6,#0x0056
mov     r7,#0x000D
orreq   r7,#0x0140
mov     r8,#0x0100
mov     r9,#0xFF00
orr     r9,#0x00FE

bl      init_env

mov     r10,#0

exec_loop:

/*
add     r0,r15,#str_6v-8-.
mov     r1,r8
mov     r2,r4
mov     r3,r5
push    {r6,r7,r9,r10}
bl      kprintf
add     r13,#16
*/

/* op = mem_read8(r_ip); */
mov     r0,r8
bl      mem_read8
mov     r11,r0

/* if (handle[op] == NULL) */
add     r2,r15,#handle-8-.
ldr     r1,[r2,r11,lsl #2]
cmp     r1,#0
beq     unknown_opcode

/* r_ip++; handle[op](); */
add     r8,#1
blx     r1

/* cycles_gone += (op == 0xCB) ? cycles0xCB[mem_read8(r_ip - 1)] : cycles[op]; */
cmp     r11,#0xCB
ldrne   r1,[r15,#cycles_addr-8-.]
bne     no_CB_op
sub     r0,r8,#1
bl      mem_read8
mov     r11,r0
ldr     r1,[r15,#cycles0xCB_addr-8-.]
no_CB_op:
ldrb    r0,[r1,r11]
add     r10,r0

/* if (cycles_gone >= CYCLE_STEP) */
cmp     r10,#CYCLE_STEP
bcc     exec_loop

mov     r0,r10
bl      update_timer
mov     r10,#0
bl      check_ints
b       exec_loop


main_int_flag_addr2:
.4byte main_int_flag

check_ints:
ldr     r2,[r15,#main_int_flag_addr2-8-.]
ldr     r0,[r2]
cmp     r0,#0
bxeq    r14

mov     r3,#0x02700000
orr     r3,#0x000E0000
ldrb    r0,[r3,#0x10F]
ldrb    r1,[r3,#0x1FF]
ands    r0,r1
bxeq    r14

mov     r1,#0x10
mov     r12,#0x60

irq_test_loop:
tst     r0,r1
bne     got_irq
sub     r12,#8
movs    r1,r1,lsr #1
bne     irq_test_loop

bx      r14

got_irq:
ldrb    r0,[r3,#0x10F]
bic     r0,r1
strb    r0,[r3,#0x10F]

stmdb   r13!,{r14}

sub     r9,#2
mov     r0,r9
mov     r1,r8
mov     r8,r12
bl      mem_write16

ldr     r2,[r15,#main_int_flag_addr2-8-.]
mov     r0,#0
str     r0,[r2]

mov     r0,#1
ldmia   r13!,{r15}


unknown_opcode:
mov     r2,r0
add     r0,r15,#str_unk_op-8-.
mov     r1,r8
bl      kprintf
b       do_gbc_panic

do_gbc_panic:
ldr     r0,[r15,#fr_af_addr-8-.]
str     r4,[r0]
ldr     r0,[r15,#fr_bc_addr-8-.]
str     r5,[r0]
ldr     r0,[r15,#fr_de_addr-8-.]
str     r6,[r0]
ldr     r0,[r15,#fr_hl_addr-8-.]
str     r7,[r0]
ldr     r0,[r15,#fr_ip_addr-8-.]
str     r8,[r0]
ldr     r0,[r15,#fr_sp_addr-8-.]
str     r9,[r0]
b       gbc_panic


has_cgb_addr:
.4byte has_cgb
cycles_addr:
.4byte cycles
cycles0xCB_addr:
.4byte cycles0xCB

fr_af_addr:
.4byte fr_af
fr_bc_addr:
.4byte fr_bc
fr_de_addr:
.4byte fr_de
fr_hl_addr:
.4byte fr_hl
fr_ip_addr:
.4byte fr_ip
fr_sp_addr:
.4byte fr_sp

str_6v:
.ascii "PC=%v AF=%v BC=%v DE=%v HL=%v SP=%v %i\n\0"

str_unk_op:
.ascii "[%V] Unbekannter Opcode %X.\n\0"

.align 4

handle:
.4byte op_nop /* 0x00 */
.4byte op_ld_bc_nn
.4byte op_ld__bc_a
.4byte op_inc_bc
.4byte op_inc_b
.4byte op_dec_b
.4byte op_ld_b_n
.4byte op_rlca
.4byte op_ld__nn_sp /* 0x08 */
.4byte op_add_hl_bc
.4byte op_ld_a__bc
.4byte op_dec_bc
.4byte op_inc_c
.4byte op_dec_c
.4byte op_ld_c_n
.4byte op_rrca
.4byte 0 /* 0x10 */
.4byte op_ld_de_nn
.4byte op_ld__de_a
.4byte op_inc_de
.4byte op_inc_d
.4byte op_dec_d
.4byte op_ld_d_n
.4byte op_rla
.4byte op_jr /* 0x18 */
.4byte op_add_hl_de
.4byte op_ld_a__de
.4byte op_dec_de
.4byte op_inc_e
.4byte op_dec_e
.4byte op_ld_e_n
.4byte op_rra
.4byte op_jrnz /* 0x20 */
.4byte op_ld_hl_nn
.4byte op_ldi__hl_a
.4byte op_inc_hl
.4byte op_inc_h
.4byte op_dec_h
.4byte op_ld_h_n
.4byte op_daa
.4byte op_jrz /* 0x28 */
.4byte op_add_hl_hl
.4byte op_ldi_a__hl
.4byte op_dec_hl
.4byte op_inc_l
.4byte op_dec_l
.4byte op_ld_l_n
.4byte op_cpl_a
.4byte op_jrnc /* 0x30 */
.4byte op_ld_sp_nn
.4byte op_ldd__hl_a
.4byte op_inc_sp
.4byte op_inc__hl
.4byte op_dec__hl
.4byte op_ld__hl_n
.4byte op_scf
.4byte op_jrc /* 0x38 */
.4byte op_add_hl_sp
.4byte op_ldd_a__hl
.4byte op_dec_sp
.4byte op_inc_a
.4byte op_dec_a
.4byte op_ld_a_n
.4byte op_ccf
.4byte op_ld_b_b /* 0x40 */
.4byte op_ld_b_c
.4byte op_ld_b_d
.4byte op_ld_b_e
.4byte op_ld_b_h
.4byte op_ld_b_l
.4byte op_ld_b__hl
.4byte op_ld_b_a
.4byte op_ld_c_b /* 0x48 */
.4byte op_ld_c_c
.4byte op_ld_c_d
.4byte op_ld_c_e
.4byte op_ld_c_h
.4byte op_ld_c_l
.4byte op_ld_c__hl
.4byte op_ld_c_a
.4byte op_ld_d_b /* 0x50 */
.4byte op_ld_d_c
.4byte op_ld_d_d
.4byte op_ld_d_e
.4byte op_ld_d_h
.4byte op_ld_d_l
.4byte op_ld_d__hl
.4byte op_ld_d_a
.4byte op_ld_e_b /* 0x58 */
.4byte op_ld_e_c
.4byte op_ld_e_d
.4byte op_ld_e_e
.4byte op_ld_e_h
.4byte op_ld_e_l
.4byte op_ld_e__hl
.4byte op_ld_e_a
.4byte op_ld_h_b /* 0x60 */
.4byte op_ld_h_c
.4byte op_ld_h_d
.4byte op_ld_h_e
.4byte op_ld_h_h
.4byte op_ld_h_l
.4byte op_ld_h__hl
.4byte op_ld_h_a
.4byte op_ld_l_b /* 0x68 */
.4byte op_ld_l_c
.4byte op_ld_l_d
.4byte op_ld_l_e
.4byte op_ld_l_h
.4byte op_ld_l_l
.4byte op_ld_l__hl
.4byte op_ld_l_a
.4byte op_ld__hl_b /* 0x70 */
.4byte op_ld__hl_c
.4byte op_ld__hl_d
.4byte op_ld__hl_e
.4byte op_ld__hl_h
.4byte op_ld__hl_l
.4byte op_halt
.4byte op_ld__hl_a
.4byte op_ld_a_b /* 0x78 */
.4byte op_ld_a_c
.4byte op_ld_a_d
.4byte op_ld_a_e
.4byte op_ld_a_h
.4byte op_ld_a_l
.4byte op_ld_a__hl
.4byte op_ld_a_a
.4byte op_add_a_b /* 0x80 */
.4byte op_add_a_c
.4byte op_add_a_d
.4byte op_add_a_e
.4byte op_add_a_h
.4byte op_add_a_l
.4byte op_add_a__hl
.4byte op_add_a_a
.4byte op_adc_a_b /* 0x88 */
.4byte op_adc_a_c
.4byte op_adc_a_d
.4byte op_adc_a_e
.4byte op_adc_a_h
.4byte op_adc_a_l
.4byte op_adc_a__hl
.4byte op_adc_a_a
.4byte op_sub_a_b /* 0x90 */
.4byte op_sub_a_c
.4byte op_sub_a_d
.4byte op_sub_a_e
.4byte op_sub_a_h
.4byte op_sub_a_l
.4byte op_sub_a__hl
.4byte op_sub_a_a
.4byte op_sbc_a_b /* 0x98 */
.4byte op_sbc_a_c
.4byte op_sbc_a_d
.4byte op_sbc_a_e
.4byte op_sbc_a_h
.4byte op_sbc_a_l
.4byte op_sbc_a__hl
.4byte op_sbc_a_a
.4byte op_and_a_b /* 0xA0 */
.4byte op_and_a_c
.4byte op_and_a_d
.4byte op_and_a_e
.4byte op_and_a_h
.4byte op_and_a_l
.4byte op_and_a__hl
.4byte op_and_a_a
.4byte op_xor_a_b /* 0xA8 */
.4byte op_xor_a_c
.4byte op_xor_a_d
.4byte op_xor_a_e
.4byte op_xor_a_h
.4byte op_xor_a_l
.4byte op_xor_a__hl
.4byte op_xor_a_a
.4byte op_or_a_b /* 0xB0 */
.4byte op_or_a_c
.4byte op_or_a_d
.4byte op_or_a_e
.4byte op_or_a_h
.4byte op_or_a_l
.4byte op_or_a__hl
.4byte op_or_a_a
.4byte op_cp_a_b /* 0xB8 */
.4byte op_cp_a_c
.4byte op_cp_a_d
.4byte op_cp_a_e
.4byte op_cp_a_h
.4byte op_cp_a_l
.4byte op_cp_a__hl
.4byte op_cp_a_a
.4byte op_retnz /* 0xC0 */
.4byte op_pop_bc
.4byte op_jpnz
.4byte op_jp
.4byte op_callnz
.4byte op_push_bc
.4byte op_add_a_n
.4byte op_rst0x00
.4byte op_retz /* 0xC8 */
.4byte op_ret
.4byte op_jpz
.4byte op_prefixCB
.4byte op_callz
.4byte op_call
.4byte op_adc_a_n
.4byte op_rst0x08
.4byte op_retnc /* 0xD0 */
.4byte op_pop_de
.4byte op_jpnc
.4byte 0
.4byte op_callnc
.4byte op_push_de
.4byte op_sub_a_n
.4byte op_rst0x10
.4byte op_retc /* 0xD8 */
.4byte op_reti
.4byte op_jpc
.4byte 0
.4byte op_callc
.4byte 0
.4byte op_sbc_a_n
.4byte op_rst0x18
.4byte op_ld__ffn_a /* 0xE0 */
.4byte op_pop_hl
.4byte op_ld__ffc_a
.4byte 0
.4byte 0
.4byte op_push_hl
.4byte op_and_a_n
.4byte op_rst0x20
.4byte op_add_sp_s /* 0xE8 */
.4byte op_jp_hl
.4byte op_ld__nn_a
.4byte 0
.4byte 0
.4byte 0
.4byte op_xor_a_n
.4byte op_rst0x28
.4byte op_ld_a__ffn /* 0xF0 */
.4byte op_pop_af
.4byte op_ld_a__ffc
.4byte op_di
.4byte 0
.4byte op_push_af
.4byte op_or_a_n
.4byte op_rst0x30
.4byte op_ld_hl_sps /* 0xF8 */
.4byte op_ld_sp_hl
.4byte op_ld_a__nn
.4byte op_ei
.4byte 0
.4byte 0
.4byte op_cp_a_n
.4byte op_rst0x38 /* 0xFF */


op_nop:
op_ld_a_a:
op_ld_b_b:
op_ld_c_c:
op_ld_d_d:
op_ld_e_e:
op_ld_h_h:
op_ld_l_l:
bx      r14


#define FS_ZERO   7
#define FS_SUB    6
#define FS_HCRY   5
#define FS_CRY    4

#define FLAG_ZERO 0x80
#define FLAG_SUB  0x40
#define FLAG_HCRY 0x20
#define FLAG_CRY  0x10


#define ld_rr_nn(vmr, armr) \
    op_ld_##vmr##_nn: \
    stmdb   r13!,{r14}; \
    mov     r0,r8; \
    bl      mem_read16; \
    add     r8,#2; \
    mov     armr,r0; \
    ldmia   r13!,{r15}

#define inc_rr(vmr, armr) \
    op_inc_##vmr: \
    add     armr,#1; \
    bic     armr,#0xFF0000; \
    bx      r14

#define dec_rr(vmr, armr) \
    op_dec_##vmr: \
    subs    armr,#1; \
    movcc   armr,armr,lsr #16; /* armr == 0xFFFFFFFF */ \
    bx      r14

#define inc_rh(vmr, armr) \
    op_inc_##vmr: \
    bic     r4,#0xFF & ~FLAG_CRY; \
    add     armr,#0x000100; \
    bic     armr,#0xFF0000; \
    tst     armr,#0x00FF00; \
    orreq   r4,#FLAG_ZERO; \
    tst     armr,#0x000F00; \
    orreq   r4,#FLAG_HCRY; \
    bx      r14

#define inc_rl(vmr, armr) \
    op_inc_##vmr: \
    bic     r4,#0xFF & ~FLAG_CRY; \
    add     armr,#0x0001; \
    tst     armr,#0x00FF; \
    subeq   armr,#0x0100; \
    orreq   r4,#FLAG_ZERO; \
    tst     armr,#0x000F; \
    orreq   r4,#FLAG_HCRY; \
    bx      r14

#define dec_rh(vmr, armr) \
    op_dec_##vmr: \
    bic     r4,#0xFF & ~FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    tst     armr,#0x0F00; \
    orreq   r4,#FLAG_HCRY; \
    subs    armr,#0x0100; \
    andcc   armr,armr,lsr #16 /* armr == 0xFFFFFF?? */; \
    tst     armr,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    bx      r14

#define dec_rl(vmr, armr) \
    op_dec_##vmr: \
    bic     r4,#0xFF & ~FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    tst     armr,#0x0F; \
    orreq   r4,#FLAG_HCRY; \
    tst     armr,#0xFF; \
    subne   armr,#0x01; \
    orreq   armr,#0xFF; \
    tst     armr,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    bx      r14

#define ld_rh_n(vmr, armr) \
    op_ld_##vmr##_n: \
    stmdb   r13!,{r14}; \
    mov     r0,r8; \
    bl      mem_read8; \
    add     r8,#1; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define ld_rl_n(vmr, armr) \
    op_ld_##vmr##_n: \
    stmdb   r13!,{r14}; \
    mov     r0,r8; \
    bl      mem_read8; \
    add     r8,#1; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

#define add_hl_rr(vmr, armr) \
    op_add_hl_##vmr: \
    bic     r4,#0xFF & ~FLAG_ZERO; \
    eor     r0,r7,armr; \
    add     r7,armr; \
    movs    r7,r7,lsl #16; \
    mov     r7,r7,lsr #16; \
    orrcs   r4,#FLAG_CRY; \
    eor     r0,r7; \
    tst     r0,#0x1000; \
    orrne   r4,#FLAG_HCRY; \
    bx      r14

#define ld_a__rr(vmr, armr) \
    op_ld_a__##vmr: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    and     r4,#0x00FF; \
    orr     r4,r0,lsl #8; \
    ldmia   r13!,{r15}

#define ld__rr_a(vmr, armr) \
    op_ld__##vmr##_a: \
    mov     r0,armr; \
    mov     r1,r4,lsr #8; \
    b       mem_write8


ld_rr_nn(bc, r5)
ld_rr_nn(de, r6)
ld_rr_nn(hl, r7)
ld_rr_nn(sp, r9)

inc_rr(bc, r5)
inc_rr(de, r6)
inc_rr(hl, r7)
inc_rr(sp, r9)

dec_rr(bc, r5)
dec_rr(de, r6)
dec_rr(hl, r7)
dec_rr(sp, r9)

inc_rh(a, r4)
inc_rh(b, r5)
inc_rl(c, r5)
inc_rh(d, r6)
inc_rl(e, r6)
inc_rh(h, r7)
inc_rl(l, r7)

dec_rh(a, r4)
dec_rh(b, r5)
dec_rl(c, r5)
dec_rh(d, r6)
dec_rl(e, r6)
dec_rh(h, r7)
dec_rl(l, r7)

ld_rh_n(a, r4)
ld_rh_n(b, r5)
ld_rl_n(c, r5)
ld_rh_n(d, r6)
ld_rl_n(e, r6)
ld_rh_n(h, r7)
ld_rl_n(l, r7)


op_inc__hl:
stmdb   r13!,{r14}
mov     r0,r7
bl      mem_read8
add     r1,r0,#1
bic     r4,#0xFF & ~FLAG_CRY
ands    r1,#0xFF
orreq   r4,#FLAG_ZERO
tst     r1,#0x0F
orreq   r4,#FLAG_HCRY
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

op_dec__hl:
stmdb   r13!,{r14}
mov     r0,r7
bl      mem_read8
subs    r1,r0,#1
andcc   r1,#0xFF
bic     r4,#0xFF & ~FLAG_CRY
orr     r4,#FLAG_SUB
orreq   r4,#FLAG_ZERO
tst     r0,#0x0F
orreq   r4,#FLAG_HCRY
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

op_ld__hl_n:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read8
add     r8,#1
mov     r1,r0
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8


op_ld__nn_sp:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read16
add     r8,#2
mov     r1,r9
ldmia   r13!,{r14}
b       mem_write16


add_hl_rr(bc, r5)
add_hl_rr(de, r6)
add_hl_rr(sp, r9)

op_add_hl_hl:
bic     r4,#0xFF & ~FLAG_ZERO
add     r7,r7
movs    r7,r7,lsl #16
mov     r7,r7,lsr #16
orrcs   r4,#FLAG_CRY
tst     r7,#0x1000
orrne   r4,#FLAG_HCRY
bx      r14


ld_a__rr(bc, r5)
ld_a__rr(de, r6)

ld__rr_a(bc, r5)
ld__rr_a(de, r6)


op_rlca:
tst     r4,#0x8000
mov     r0,r4,lsl #1
moveq   r4,#0x0000
movne   r4,#0x0100 | FLAG_CRY
and     r0,#0xFE00
orr     r4,r0
bx      r14


op_rrca:
mov     r0,r4,lsr #9
ands    r4,#0x0100
mov     r4,r4,lsr #8 - FS_CRY
orrne   r4,#0x8000
orr     r4,r0,lsl #8
bx      r14


op_rla:
mov     r0,r4,lsl #1
and     r4,#FLAG_CRY
mov     r4,r4,lsl #8 - FS_CRY
tst     r0,#0x10000
orrne   r4,#FLAG_CRY
and     r0,#0xFE00
orr     r4,r0
bx      r14


op_rra:
movs    r0,r4,lsr #9
and     r4,#FLAG_CRY
mov     r4,r4,lsl #15 - FS_CRY
orrcs   r4,#FLAG_CRY
orr     r4,r0,lsl #8
bx      r14


op_jr:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read8
mov     r0,r0,lsl #24
add     r8,r0,asr #24
add     r8,#1
// Zur Sicherheit, könnte für Optimierungen weg
mvn     r0,#0
and     r8,r0,lsr #16
ldmia   r13!,{r15}

op_jrnz:
tst     r4,#FLAG_ZERO
beq     op_jr
add     r8,#1
bx      r14

op_jrz:
tst     r4,#FLAG_ZERO
bne     op_jr
add     r8,#1
bx      r14

op_jrnc:
tst     r4,#FLAG_CRY
beq     op_jr
add     r8,#1
bx      r14

op_jrc:
tst     r4,#FLAG_CRY
bne     op_jr
add     r8,#1
bx      r14


op_ldi__hl_a:
mov     r0,r7
add     r7,#1
bic     r7,#0xFF0000
mov     r1,r4,lsr #8
b       mem_write8

op_ldi_a__hl:
stmdb   r13!,{r14}
mov     r0,r7
bl      mem_read8
add     r7,#1
bic     r7,#0xFF0000
and     r4,#0x00FF
orr     r4,r0,lsl #8
ldmia   r13!,{r15}

op_ldd__hl_a:
mov     r0,r7
subs    r7,#1
andcc   r7,r7,lsr #16 /* r7 == 0xFFFFFFFF */
mov     r1,r4,lsr #8
b       mem_write8

op_ldd_a__hl:
stmdb   r13!,{r14}
mov     r0,r7
bl      mem_read8
subs    r7,#1
andcc   r7,r7,lsr #16 /* r7 == 0xFFFFFFFF */
and     r4,#0x00FF
orr     r4,r0,lsl #8
ldmia   r13!,{r15}


daa_table_addr:
.4byte daa_table

op_daa:
ldr     r0,[r15,#daa_table_addr-8-.]
and     r1,r4,#FLAG_SUB | FLAG_HCRY | FLAG_CRY // 0x70
add     r1,r0,r1,lsl #5 // 0x70 << 5 == 0x700 * 2
and     r0,r4,#0xFF00 // 0xFF00
orr     r1,r0,lsr #7 // 0xFF00 >> 7 = 0x0FF * 2
ldrh    r0,[r1]
and     r4,#FLAG_SUB
orr     r4,r0
bx      r14


op_cpl_a:
eor     r4,#0xFF00
orr     r4,#FLAG_SUB | FLAG_HCRY
bx      r14


op_scf:
bic     r4,#FLAG_SUB | FLAG_HCRY | 0x0F
orr     r4,#FLAG_CRY
bx      r14

op_ccf:
bic     r4,#FLAG_SUB | FLAG_HCRY | 0x0F
eor     r4,#FLAG_CRY
bx      r14


#define in_armr_ld_rh_rl(vmr1, vmr2, armr) \
    op_ld_##vmr1##_##vmr2: \
    and     armr,#0x00FF; \
    orr     armr,armr,lsl #8; \
    bx      r14

#define in_armr_ld_rl_rh(vmr1, vmr2, armr) \
    op_ld_##vmr1##_##vmr2: \
    and     armr,#0xFF00; \
    orr     armr,armr,lsr #8; \
    bx      r14

#define ld_rh_rh(vmr1, vmr2, armr1, armr2) \
    op_ld_##vmr1##_##vmr2: \
    and     armr1,#0x00FF; \
    and     r0,armr2,#0xFF00; \
    orr     armr1,r0; \
    bx      r14

#define ld_rh_rl(vmr1, vmr2, armr1, armr2) \
    op_ld_##vmr1##_##vmr2: \
    and     armr1,#0x00FF; \
    and     r0,armr2,#0x00FF; \
    orr     armr1,r0,lsl #8; \
    bx      r14

#define ld_rl_rh(vmr1, vmr2, armr1, armr2) \
    op_ld_##vmr1##_##vmr2: \
    and     armr1,#0xFF00; \
    orr     armr1,armr2,lsr #8; \
    bx      r14

#define ld_rl_rl(vmr1, vmr2, armr1, armr2) \
    op_ld_##vmr1##_##vmr2: \
    and     armr1,#0xFF00; \
    and     r0,armr2,#0x00FF; \
    orr     armr1,r0; \
    bx      r14

#define ld_rh__hl(vmr, armr) \
    op_ld_##vmr##__hl: \
    stmdb   r13!,{r14}; \
    mov     r0,r7; \
    bl      mem_read8; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define ld_rl__hl(vmr, armr) \
    op_ld_##vmr##__hl: \
    stmdb   r13!,{r14}; \
    mov     r0,r7; \
    bl      mem_read8; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

#define ld__hl_rh(vmr, armr) \
    op_ld__hl_##vmr: \
    mov     r0,r7; \
    mov     r1,armr,lsr #8; \
    b       mem_write8

#define ld__hl_rl(vmr, armr) \
    op_ld__hl_##vmr: \
    mov     r0,r7; \
    and     r1,armr,#0x00FF; \
    b       mem_write8


ld_rh_rh(a, b, r4, r5)
ld_rh_rl(a, c, r4, r5)
ld_rh_rh(a, d, r4, r6)
ld_rh_rl(a, e, r4, r6)
ld_rh_rh(a, h, r4, r7)
ld_rh_rl(a, l, r4, r7)
ld_rh_rh(b, a, r5, r4)
in_armr_ld_rh_rl(b, c, r5)
ld_rh_rh(b, d, r5, r6)
ld_rh_rl(b, e, r5, r6)
ld_rh_rh(b, h, r5, r7)
ld_rh_rl(b, l, r5, r7)
ld_rl_rh(c, a, r5, r4)
in_armr_ld_rl_rh(c, b, r5)
ld_rl_rh(c, d, r5, r6)
ld_rl_rl(c, e, r5, r6)
ld_rl_rh(c, h, r5, r7)
ld_rl_rl(c, l, r5, r7)
ld_rh_rh(d, a, r6, r4)
ld_rh_rh(d, b, r6, r5)
ld_rh_rl(d, c, r6, r5)
in_armr_ld_rh_rl(d, e, r6)
ld_rh_rh(d, h, r6, r7)
ld_rh_rl(d, l, r6, r7)
ld_rl_rh(e, a, r6, r4)
ld_rl_rh(e, b, r6, r5)
ld_rl_rl(e, c, r6, r5)
in_armr_ld_rl_rh(e, d, r6)
ld_rl_rh(e, h, r6, r7)
ld_rl_rl(e, l, r6, r7)
ld_rh_rh(h, a, r7, r4)
ld_rh_rh(h, b, r7, r5)
ld_rh_rl(h, c, r7, r5)
ld_rh_rh(h, d, r7, r6)
ld_rh_rl(h, e, r7, r6)
in_armr_ld_rh_rl(h, l, r7)
ld_rl_rh(l, a, r7, r4)
ld_rl_rh(l, b, r7, r5)
ld_rl_rl(l, c, r7, r5)
ld_rl_rh(l, d, r7, r6)
ld_rl_rl(l, e, r7, r6)
in_armr_ld_rl_rh(l, h, r7)

ld_rh__hl(a, r4)
ld_rh__hl(b, r5)
ld_rl__hl(c, r5)
ld_rh__hl(d, r6)
ld_rl__hl(e, r6)
ld_rh__hl(h, r7)
ld_rl__hl(l, r7)
ld__hl_rh(a, r4)
ld__hl_rh(b, r5)
ld__hl_rl(c, r5)
ld__hl_rh(d, r6)
ld__hl_rl(e, r6)
ld__hl_rh(h, r7)
ld__hl_rl(l, r7)


op_halt:
stmdb   r13!,{r14}
halt_loop:
mov     r0,#HALT_CYCLE_STEP
bl      update_timer
bl      check_ints
cmp     r0,#0
beq     halt_loop
ldmia   r13!,{r15}


#define add_a_r(vmr, armr, is_high) \
    op_add_a_##vmr: \
    .if is_high; \
    mov     r0,armr,lsl #16; /* 0x000012__ -> 0x12__0000 */ \
    and     r0,#0xFF000000;  /* 0x12__0000 -> 0x12000000 */ \
    .else; \
    mov     r0,armr,lsl #24; /* 0x0000__12 -> 0x12000000 */ \
    .endif; \
    mov     r1,r0,lsl #4;    /* 0x12000000 -> 0x20000000 */ \
    mov     r2,r4,lsr #8;    /* 0x000034__ -> 0x00000034 */ \
    adds    r0,r2,lsl #24;   /* 0x00000034 -> 0x34000000 + 0x12000000 -> 0x56000000 */ \
    mov     r2,r2,lsl #28;   /* 0x00000034 -> 0x40000000 */ \
    mov     r4,r0,lsr #16;   /* 0x56000000 -> 0x00005600 */ \
    orreq   r4,#FLAG_ZERO; \
    orrcs   r4,#FLAG_CRY; \
    adds    r1,r2;           /* 0x20000000 + 0x40000000 = 0x60000000 */ \
    orrcs   r4,#FLAG_HCRY; \
    bx      r14

#define add_a_m(name, armr, inc) \
    op_add_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    mov     r0,r0,lsl #24; \
    mov     r1,r0,lsl #4; \
    mov     r2,r4,lsr #8; \
    adds    r0,r2,lsl #24; \
    mov     r2,r2,lsl #28; \
    mov     r4,r0,lsr #16; \
    orreq   r4,#FLAG_ZERO; \
    orrcs   r4,#FLAG_CRY; \
    adds    r1,r2; \
    orrcs   r4,#FLAG_HCRY; \
    ldmia   r13!,{r15}


add_a_r(b, r5, true)
add_a_r(c, r5, false)
add_a_r(d, r6, true)
add_a_r(e, r6, false)
add_a_r(h, r7, true)
add_a_r(l, r7, false)

add_a_m(_hl, r7, false)
add_a_m(n, r8, true)

op_add_a_a:
and     r0,r4,#0x0000FF00
movs    r0,r0,lsl #17
mov     r4,r0,lsr #16
orreq   r4,#FLAG_ZERO
orrcs   r4,#FLAG_CRY
movs    r0,r0,lsl #4
orrcs   r4,#FLAG_HCRY
bx      r14


#define adc_a_r(vmr, armr, is_high) \
    op_adc_a_##vmr: \
    .if is_high; \
    mov     r0,armr,lsr #8; \
    .else; \
    and     r0,armr,#0xFF; \
    .endif; \
    and     r1,r0,#0xF; \
    mov     r2,r4,lsr #8; \
    movs    r3,r4,lsr #FS_CRY + 1; /* Carry-Flag ins CPSR übernehmen */ \
    addcs   r1,#1; \
    adc     r0,r2; \
    and     r2,#0xF; \
    mov     r4,r0,lsl #8; \
    ands    r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    cmp     r0,#0x100; \
    orrcs   r4,#FLAG_CRY; \
    add     r1,r2; \
    cmp     r1,#0x10; \
    orrcs   r4,#FLAG_HCRY; \
    bx      r14

#define adc_a_m(name, armr, inc) \
    op_adc_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    and     r1,r0,#0xF; \
    mov     r2,r4,lsr #8; \
    movs    r3,r4,lsr #FS_CRY + 1; \
    addcs   r1,#1; \
    adc     r0,r2; \
    and     r2,#0xF; \
    mov     r4,r0,lsl #8; \
    ands    r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    cmp     r0,#0x100; \
    orrcs   r4,#FLAG_CRY; \
    add     r1,r2; \
    cmp     r1,#0x10; \
    orrcs   r4,#FLAG_HCRY; \
    ldmia   r13!,{r15}


adc_a_r(b, r5, true)
adc_a_r(c, r5, false)
adc_a_r(d, r6, true)
adc_a_r(e, r6, false)
adc_a_r(h, r7, true)
adc_a_r(l, r7, false)

adc_a_m(_hl, r7, false)
adc_a_m(n, r8, true)

op_adc_a_a:
and     r0,r4,#0x0000FF00
tst     r4,#FLAG_CRY
orrne   r0,#0x80
movs    r0,r0,lsl #17
mov     r4,r0,lsr #16
orreq   r4,#FLAG_ZERO
orrcs   r4,#FLAG_CRY
movs    r0,r0,lsl #4
orrcs   r4,#FLAG_HCRY
bx      r14


#define sub_a_r(vmr, armr, get_op, get_parm) \
    op_sub_a_##vmr: \
    mov     r0,r4,lsr #8; \
    and     r1,r0,#0xF; \
    get_op  r2,armr,get_parm; \
    subs    r0,r2; \
    and     r0,#0xFF; \
    and     r2,#0xF; \
    mov     r4,r0,lsl #8; \
    orreq   r4,#FLAG_ZERO; \
    orrcc   r4,#FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    cmp     r1,r2; \
    orrcc   r4,#FLAG_HCRY; \
    bx      r14

#define sub_a_m(name, armr, inc) \
    op_sub_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    mov     r2,r4,lsr #8; \
    and     r1,r0,#0xF; \
    subs    r0,r2,r0; \
    and     r0,#0xFF; \
    and     r2,#0xF; \
    mov     r4,r0,lsl #8; \
    orreq   r4,#FLAG_ZERO; \
    orrcc   r4,#FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    cmp     r2,r1; \
    orrcc   r4,#FLAG_HCRY; \
    ldmia   r13!,{r15}

sub_a_r(b, r5, mov, lsr #8)
sub_a_r(c, r5, and, #0xFF)
sub_a_r(d, r6, mov, lsr #8)
sub_a_r(e, r6, and, #0xFF)
sub_a_r(h, r7, mov, lsr #8)
sub_a_r(l, r7, and, #0xFF)

sub_a_m(_hl, r7, false)
sub_a_m(n, r8, true)

op_sub_a_a:
mov     r4,#0x0000 | FLAG_ZERO | FLAG_SUB
bx      r14


#define sbc_a_r(vmr, armr, get_op, get_parm) \
    op_sbc_a_##vmr: \
    mov     r0,r4,lsr #8; \
    and     r1,r0,#0xF; \
    get_op  r2,armr,get_parm; \
    and     r3,r2,#0xF; \
    tst     r4,#FLAG_CRY; \
    addne   r2,#1; \
    addne   r3,#1; \
    subs    r0,r2; \
    and     r0,#0xFF; \
    mov     r4,r0,lsl #8; \
    orreq   r4,#FLAG_ZERO; \
    orrcc   r4,#FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    cmp     r1,r3; \
    orrcc   r4,#FLAG_HCRY; \
    bx      r14

#define sbc_a_m(name, armr, inc) \
    op_sbc_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    mov     r2,r4,lsr #8; \
    and     r1,r0,#0xF; \
    tst     r4,#FLAG_CRY; \
    addne   r0,#0x1; \
    addne   r1,#0x1; \
    subs    r0,r2,r0; \
    and     r0,#0xFF; \
    and     r2,#0xF; \
    mov     r4,r0,lsl #8; \
    orreq   r4,#FLAG_ZERO; \
    orrcc   r4,#FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    cmp     r2,r1; \
    orrcc   r4,#FLAG_HCRY; \
    ldmia   r13!,{r15}

sbc_a_r(b, r5, mov, lsr #8)
sbc_a_r(c, r5, and, #0xFF)
sbc_a_r(d, r6, mov, lsr #8)
sbc_a_r(e, r6, and, #0xFF)
sbc_a_r(h, r7, mov, lsr #8)
sbc_a_r(l, r7, and, #0xFF)

sbc_a_m(_hl, r7, false)
sbc_a_m(n, r8, true)

op_sbc_a_a:
tst     r4,#FLAG_CRY
movne   r4,#0xFF00
orrne   r4,#FLAG_ZERO | FLAG_SUB | FLAG_HCRY | FLAG_CRY
moveq   r4,#FLAG_ZERO | FLAG_SUB
bx      r14


#define and_a_r(vmr, armr, shift) \
    op_and_a_##vmr: \
    and     r4,#0xFF00; \
    ands    r4,armr,lsl shift; \
    orrne   r4,#FLAG_HCRY; \
    orreq   r4,#FLAG_HCRY | FLAG_ZERO; \
    bx      r14

#define and_a_m(name, armr, inc) \
    op_and_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    and     r4,#0xFF00; \
    ands    r4,r0,lsl #8; \
    orrne   r4,#FLAG_HCRY; \
    orreq   r4,#FLAG_HCRY | FLAG_ZERO; \
    ldmia   r13!,{r15}

and_a_r(b, r5, #0)
and_a_r(c, r5, #8)
and_a_r(d, r6, #0)
and_a_r(e, r6, #8)
and_a_r(h, r7, #0)
and_a_r(l, r7, #8)

and_a_m(_hl, r7, false)
and_a_m(n, r8, true)

op_and_a_a:
ands    r4,#0xFF00
orrne   r4,#FLAG_HCRY
orreq   r4,#FLAG_HCRY | FLAG_ZERO
bx      r14


#define xor_a_r(vmr, armr, shift) \
    op_xor_a_##vmr: \
    eor     r4,armr,lsl shift; \
    ands    r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    bx      r14

#define xor_a_m(name, armr, inc) \
    op_xor_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    eor     r4,r0,lsl #8; \
    ands    r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    ldmia   r13!,{r15}

xor_a_r(b, r5, #0)
xor_a_r(c, r5, #8)
xor_a_r(d, r6, #0)
xor_a_r(e, r6, #8)
xor_a_r(h, r7, #0)
xor_a_r(l, r7, #8)

xor_a_m(_hl, r7, false)
xor_a_m(n, r8, true)

op_xor_a_a:
mov     r4,#0x0000 | FLAG_ZERO
bx      r14


#define or_a_r(vmr, armr, shift) \
    op_or_a_##vmr: \
    orr     r4,armr,lsl shift; \
    ands    r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    bx      r14

#define or_a_m(name, armr, inc) \
    op_or_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    orr     r4,r0,lsl #8; \
    ands    r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    ldmia   r13!,{r15}


or_a_r(b, r5, #0)
or_a_r(c, r5, #8)
or_a_r(d, r6, #0)
or_a_r(e, r6, #8)
or_a_r(h, r7, #0)
or_a_r(l, r7, #8)

or_a_m(_hl, r7, false)
or_a_m(n, r8, true)

op_or_a_a:
ands    r4,#0xFF00
orreq   r4,#FLAG_ZERO
bx      r14


#define cp_a_r(vmr, armr, get_op, get_parm) \
    op_cp_a_##vmr: \
    mov     r0,r4,lsr #8; \
    and     r1,r0,#0xF; \
    get_op  r2,armr,get_parm; \
    subs    r0,r2; \
    and     r0,#0xFF; \
    and     r2,#0xF; \
    and     r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    orrcc   r4,#FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    cmp     r1,r2; \
    orrcc   r4,#FLAG_HCRY; \
    bx      r14

#define cp_a_m(name, armr, inc) \
    op_cp_a_##name: \
    stmdb   r13!,{r14}; \
    mov     r0,armr; \
    bl      mem_read8; \
    .if inc; \
    add     armr,#1; \
    .endif; \
    mov     r2,r4,lsr #8; \
    and     r1,r0,#0xF; \
    subs    r0,r2,r0; \
    and     r0,#0xFF; \
    and     r2,#0xF; \
    and     r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    orrcc   r4,#FLAG_CRY; \
    orr     r4,#FLAG_SUB; \
    cmp     r2,r1; \
    orrcc   r4,#FLAG_HCRY; \
    ldmia   r13!,{r15}


cp_a_r(b, r5, mov, lsr #8)
cp_a_r(c, r5, and, #0xFF)
cp_a_r(d, r6, mov, lsr #8)
cp_a_r(e, r6, and, #0xFF)
cp_a_r(h, r7, mov, lsr #8)
cp_a_r(l, r7, and, #0xFF)

cp_a_m(_hl, r7, false)
cp_a_m(n, r8, true)

op_cp_a_a:
and     r4,#0xFF00
orr     r4,#FLAG_ZERO | FLAG_SUB
bx      r14


op_ret:
stmdb   r13!,{r14}
mov     r0,r9
bl      mem_read16
add     r9,#2
mov     r8,r0
ldmia   r13!,{r15}

op_retnz:
tst     r4,#FLAG_ZERO
beq     op_ret
bx      r14

op_retz:
tst     r4,#FLAG_ZERO
bne     op_ret
bx      r14

op_retnc:
tst     r4,#FLAG_CRY
beq     op_ret
bx      r14

op_retc:
tst     r4,#FLAG_CRY
bne     op_ret
bx      r14


#define pop_rr(vmr, armr) \
    op_pop_##vmr: \
    stmdb   r13!,{r14}; \
    mov     r0,r9; \
    bl      mem_read16; \
    add     r9,#2; \
    mov     armr,r0; \
    ldmia   r13!,{r15}

#define push_rr(vmr, armr) \
    op_push_##vmr: \
    sub     r9,#2; \
    mov     r0,r9; \
    mov     r1,armr; \
    b       mem_write16

pop_rr(af, r4)
pop_rr(bc, r5)
pop_rr(de, r6)
pop_rr(hl, r7)

push_rr(af, r4)
push_rr(bc, r5)
push_rr(de, r6)
push_rr(hl, r7)


op_jp_hl:
mov     r8,r7
bx      r14

op_jp:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read16
mov     r8,r0
ldmia   r13!,{r15}

op_jpnz:
tst     r4,#FLAG_ZERO
beq     op_jp
add     r8,#2
bx      r14

op_jpz:
tst     r4,#FLAG_ZERO
bne     op_jp
add     r8,#2
bx      r14

op_jpnc:
tst     r4,#FLAG_CRY
beq     op_jp
add     r8,#2
bx      r14

op_jpc:
tst     r4,#FLAG_CRY
bne     op_jp
add     r8,#2
bx      r14

op_call:
stmdb   r13!,{r14}
sub     r9,#2
mov     r0,r9
add     r1,r8,#2
bl      mem_write16
mov     r0,r8
bl      mem_read16
mov     r8,r0
ldmia   r13!,{r15}

op_callnz:
tst     r4,#FLAG_ZERO
beq     op_call
add     r8,#2
bx      r14

op_callz:
tst     r4,#FLAG_ZERO
bne     op_call
add     r8,#2
bx      r14

op_callnc:
tst     r4,#FLAG_CRY
beq     op_call
add     r8,#2
bx      r14

op_callc:
tst     r4,#FLAG_CRY
bne     op_call
add     r8,#2
bx      r14


#define rst(ip, ipnum) \
    op_rst##ip: \
    sub     r9,#2; \
    mov     r0,r9; \
    mov     r1,r8; \
    mov     r8,ipnum; \
    b       mem_write16

rst(0x00, #0x00)
rst(0x08, #0x08)
rst(0x10, #0x10)
rst(0x18, #0x18)
rst(0x20, #0x20)
rst(0x28, #0x28)
rst(0x30, #0x30)
rst(0x38, #0x38)


op_ld__ffn_a:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read8
add     r8,#1
orr     r0,#0xFF00
mov     r1,r4,lsr #8
ldmia   r13!,{r14}
b       mem_write8

op_ld__ffc_a:
and     r0,r5,#0xFF
orr     r0,#0xFF00
mov     r1,r4,lsr #8
b       mem_write8

op_ld_a__ffn:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read8
add     r8,#1
orr     r0,#0xFF00
bl      mem_read8
and     r4,#0xFF
orr     r4,r0,lsl #8
ldmia   r13!,{r15}

op_ld_a__ffc:
stmdb   r13!,{r14}
and     r0,r5,#0xFF
orr     r0,#0xFF00
bl      mem_read8
and     r4,#0xFF
orr     r4,r0,lsr #8
ldmia   r13!,{r15}


op_add_sp_s:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read8
add     r8,#1
mov     r2,r0,lsl #24
eor     r0,r9,r2,asr #24
add     r9,r2,asr #24
and     r4,#0xFF00
cmp     r9,#0x10000
orrcs   r4,#FLAG_CRY
mvncs   r1,#0
andcs   r9,r1,lsr #16
eor     r0,r9
tst     r0,#0x1000
orrne   r4,#FS_HCRY
ldmia   r13!,{r15}

op_ld_hl_sps:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read8
add     r8,#1
mov     r2,r0,lsl #24
eor     r0,r9,r2,asr #24
add     r7,r9,r2,asr #24
and     r4,#0xFF00
cmp     r7,#0x10000
orrcs   r4,#FLAG_CRY
mvncs   r1,#0
andcs   r7,r1,lsr #16
eor     r0,r7
tst     r0,#0x1000
orrne   r4,#FS_HCRY
ldmia   r13!,{r15}

op_ld_sp_hl:
mov     r9,r7
bx      r14


op_ld__nn_a:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read16
add     r8,#2
mov     r1,r4,lsr #8
ldmia   r13!,{r14}
b       mem_write8

op_ld_a__nn:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read16
add     r8,#2
bl      mem_read8
and     r4,#0x00FF
orr     r4,r0,lsl #8
ldmia   r13!,{r15}


main_int_flag_addr:
.4byte main_int_flag

op_di:
ldr     r0,[r15,#main_int_flag_addr-8-.]
mov     r1,#0
str     r1,[r0]
bx      r14

op_ei:
ldr     r0,[r15,#main_int_flag_addr-8-.]
mov     r1,#1
str     r1,[r0]
bx      r14

op_reti:
ldr     r0,[r15,#main_int_flag_addr-8-.]
mov     r1,#1
str     r1,[r0]
b       op_ret


prefixCB_gen_tables:
prefixCB_bit_table:
.4byte opCB_bit_b
.4byte opCB_bit_c
.4byte opCB_bit_d
.4byte opCB_bit_e
.4byte opCB_bit_h
.4byte opCB_bit_l
.4byte opCB_bit__hl
.4byte opCB_bit_a
prefixCB_res_table:
.4byte opCB_res_b
.4byte opCB_res_c
.4byte opCB_res_d
.4byte opCB_res_e
.4byte opCB_res_h
.4byte opCB_res_l
.4byte opCB_res__hl
.4byte opCB_res_a
prefixCB_set_table:
.4byte opCB_set_b
.4byte opCB_set_c
.4byte opCB_set_d
.4byte opCB_set_e
.4byte opCB_set_h
.4byte opCB_set_l
.4byte opCB_set__hl
.4byte opCB_set_a

op_prefixCB:
stmdb   r13!,{r14}
mov     r0,r8
bl      mem_read8
add     r8,#1
ands    r3,r0,#0xC0
beq     opCB_misc
and     r1,r0,#0x38
mov     r1,r1,lsr #3
mov     r2,#1
mov     r2,r2,lsl r1
and     r1,r0,#0x07
sub     r3,#0x40
orr     r1,r3,lsr #3
add     r3,r15,#prefixCB_gen_tables-8-.
ldr     r1,[r3,r1,lsl #2]
bx      r1

#define opCB_bit_r(vmr, armr, shift) \
    opCB_bit_##vmr: \
    tst     armr,r2,lsl shift; \
    bic     r4,#0xFF & ~FLAG_CRY; \
    orreq   r4,#FLAG_ZERO | FLAG_HCRY; \
    orrne   r4,#FLAG_HCRY; \
    ldmia   r13!,{r15}

opCB_bit_r(a, r4, #8)
opCB_bit_r(b, r5, #8)
opCB_bit_r(c, r5, #0)
opCB_bit_r(d, r6, #8)
opCB_bit_r(e, r6, #0)
opCB_bit_r(h, r7, #8)
opCB_bit_r(l, r7, #0)

opCB_bit__hl:
stmdb   r13!,{r2}
mov     r0,r7
bl      mem_read8
ldmia   r13!,{r2}
tst     r0,r2
bic     r4,#0xFF & ~FLAG_CRY
orreq   r4,#FLAG_ZERO | FLAG_HCRY
orrne   r4,#FLAG_HCRY
ldmia   r13!,{r15}

#define opCB_res_r(vmr, armr, shift) \
    opCB_res_##vmr: \
    bic     armr,r2,lsl shift; \
    ldmia   r13!,{r15}

opCB_res_r(a, r4, #8)
opCB_res_r(b, r5, #8)
opCB_res_r(c, r5, #0)
opCB_res_r(d, r6, #8)
opCB_res_r(e, r6, #0)
opCB_res_r(h, r7, #8)
opCB_res_r(l, r7, #0)

opCB_res__hl:
stmdb   r13!,{r2}
mov     r0,r7
bl      mem_read8
ldmia   r13!,{r2,r14}
bic     r0,r2
mov     r1,r0
mov     r0,r7
b       mem_write8

#define opCB_set_r(vmr, armr, shift) \
    opCB_set_##vmr: \
    orr     armr,r2,lsl shift; \
    ldmia   r13!,{r15}

opCB_set_r(a, r4, #8)
opCB_set_r(b, r5, #8)
opCB_set_r(c, r5, #0)
opCB_set_r(d, r6, #8)
opCB_set_r(e, r6, #0)
opCB_set_r(h, r7, #8)
opCB_set_r(l, r7, #0)

opCB_set__hl:
stmdb   r13!,{r2}
mov     r0,r7
bl      mem_read8
ldmia   r13!,{r2,r14}
orr     r0,r2
mov     r1,r0
mov     r0,r7
b       mem_write8


handle0xCB:
.4byte op_rlc_b /* 0x00 */
.4byte op_rlc_c
.4byte op_rlc_d
.4byte op_rlc_e
.4byte op_rlc_h
.4byte op_rlc_l
.4byte op_rlc__hl
.4byte op_rlc_a
.4byte op_rrc_b /* 0x08 */
.4byte op_rrc_c
.4byte op_rrc_d
.4byte op_rrc_e
.4byte op_rrc_h
.4byte op_rrc_l
.4byte op_rrc__hl
.4byte op_rrc_a
.4byte op_rl_b /* 0x10 */
.4byte op_rl_c
.4byte op_rl_d
.4byte op_rl_e
.4byte op_rl_h
.4byte op_rl_l
.4byte op_rl__hl
.4byte op_rl_a
.4byte op_rr_b /* 0x18 */
.4byte op_rr_c
.4byte op_rr_d
.4byte op_rr_e
.4byte op_rr_h
.4byte op_rr_l
.4byte op_rr__hl
.4byte op_rr_a
.4byte op_sla_b /* 0x20 */
.4byte op_sla_c
.4byte op_sla_d
.4byte op_sla_e
.4byte op_sla_h
.4byte op_sla_l
.4byte op_sla__hl
.4byte op_sla_a
.4byte op_sra_b /* 0x28 */
.4byte op_sra_c
.4byte op_sra_d
.4byte op_sra_e
.4byte op_sra_h
.4byte op_sra_l
.4byte op_sra__hl
.4byte op_sra_a
.4byte op_swap_b /* 0x30 */
.4byte op_swap_c
.4byte op_swap_d
.4byte op_swap_e
.4byte op_swap_h
.4byte op_swap_l
.4byte op_swap__hl
.4byte op_swap_a
.4byte op_srl_b /* 0x38 */
.4byte op_srl_c
.4byte op_srl_d
.4byte op_srl_e
.4byte op_srl_h
.4byte op_srl_l
.4byte op_srl__hl
.4byte op_srl_a /* 0x3F */

opCB_misc:
and     r1,r0,#0x3F
add     r2,r15,#handle0xCB-8-.
ldr     r0,[r2,r1,lsl #2]
bx      r0

#define rlc_rh(vmr, armr) \
    op_rlc_##vmr: \
    mov     r0,armr,lsr #7; \
    tst     r0,#0x100; \
    and     r0,#0xFE; \
    and     r4,#0xFF00; \
    orrne   r4,#FLAG_CRY; \
    orrne   r0,#0x01; \
    tsteq   r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define rlc_rl(vmr, armr) \
    op_rlc_##vmr: \
    mov     r0,armr,lsl #1; \
    tst     r0,#0x100; \
    and     r0,#0xFE; \
    and     r4,#0xFF00; \
    orrne   r4,#FLAG_CRY; \
    orrne   r0,#0x01; \
    tsteq   r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

rlc_rh(a, r4)
rlc_rh(b, r5)
rlc_rl(c, r5)
rlc_rh(d, r6)
rlc_rl(e, r6)
rlc_rh(h, r7)
rlc_rl(l, r7)

op_rlc__hl:
mov     r0,r7
bl      mem_read8
tst     r0,#0x80
and     r0,#0x7F
and     r4,#0xFF00
orrne   r4,#FLAG_CRY
orrne   r0,#0x80000000
tsteq   r0,#0x7F
orreq   r4,#FLAG_ZERO
mov     r1,r0,ror #31
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

#define rrc_rh(vmr, armr) \
    op_rrc_##vmr: \
    movs    r0,armr,lsr #9; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    orrcs   r0,#0x80; \
    tsteq   r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define rrc_rl(vmr, armr) \
    op_rrc_##vmr: \
    movs    r0,armr,lsr #1; \
    and     r0,#0x7F; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    orrcs   r0,#0x80; \
    tsteq   r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

rrc_rh(a, r4)
rrc_rh(b, r5)
rrc_rl(c, r5)
rrc_rh(d, r6)
rrc_rl(e, r6)
rrc_rh(h, r7)
rrc_rl(l, r7)

op_rrc__hl:
mov     r0,r7
bl      mem_read8
tst     r0,#0x01
and     r0,#0xFE
and     r4,#0xFF00
orrne   r4,#FLAG_CRY
orrne   r0,#0x100
tsteq   r0,#0x0FE
orreq   r4,#FLAG_ZERO
mov     r1,r0,lsr #1
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

#define rl_rh(vmr, armr) \
    op_rl_##vmr: \
    mov     r0,armr,lsr #7; \
    tst     r0,#0x100; \
    and     r0,#0xFE; \
    and     r1,r4,#FLAG_CRY; \
    orr     r0,r1,lsr #FS_CRY; \
    and     r4,#0xFF00; \
    orrne   r4,#FLAG_CRY; \
    tst     r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define rl_rl(vmr, armr) \
    op_rl_##vmr: \
    mov     r0,armr,lsl #1; \
    tst     r0,#0x100; \
    and     r0,#0xFE; \
    and     r1,r4,#FLAG_CRY; \
    orr     r0,r1,lsr #FS_CRY; \
    and     r4,#0xFF00; \
    orrne   r4,#FLAG_CRY; \
    tst     r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

rl_rh(a, r4)
rl_rh(b, r5)
rl_rl(c, r5)
rl_rh(d, r6)
rl_rl(e, r6)
rl_rh(h, r7)
rl_rl(l, r7)

op_rl__hl:
mov     r0,r7
bl      mem_read8
tst     r0,#0x80
and     r0,#0x7F
and     r2,r4,#FLAG_CRY
orr     r0,r2,lsl #31 - FS_CRY
and     r4,#0xFF00
orrne   r4,#FLAG_CRY
movs    r1,r0,ror #31
orreq   r4,#FLAG_ZERO
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

#define rr_rh(vmr, armr) \
    op_rr_##vmr: \
    movs    r0,armr,lsr #9; \
    and     r1,r4,#FLAG_CRY; \
    orr     r0,r1,lsl #7 - FS_CRY; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    tst     r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define rr_rl(vmr, armr) \
    op_rr_##vmr: \
    movs    r0,armr,lsr #1; \
    and     r0,#0x7F; \
    and     r1,r4,#FLAG_CRY; \
    orr     r0,r1,lsl #7 - FS_CRY; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    tst     r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

rr_rh(a, r4)
rr_rh(b, r5)
rr_rl(c, r5)
rr_rh(d, r6)
rr_rl(e, r6)
rr_rh(h, r7)
rr_rl(l, r7)

op_rr__hl:
mov     r0,r7
bl      mem_read8
and     r0,#0xFE
and     r2,r4,#FLAG_CRY
orr     r0,r2,lsl #8 - FS_CRY
and     r4,#0xFF00
movs    r1,r0,lsr #1
orrcs   r4,#FLAG_CRY
orreq   r4,#FLAG_ZERO
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

#define sla_rh(vmr, armr) \
    op_sla_##vmr: \
    mov     r0,armr,lsr #7; \
    tst     r0,#0x100; \
    and     r4,#0xFF00; \
    orrne   r4,#FLAG_CRY; \
    ands    r0,#0xFE; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define sla_rl(vmr, armr) \
    op_sla_##vmr: \
    mov     r0,armr,lsl #1; \
    tst     r0,#0x100; \
    and     r4,#0xFF00; \
    orrne   r4,#FLAG_CRY; \
    ands    r0,#0xFE; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

sla_rh(a, r4)
sla_rh(b, r5)
sla_rl(c, r5)
sla_rh(d, r6)
sla_rl(e, r6)
sla_rh(h, r7)
sla_rl(l, r7)

op_sla__hl:
mov     r0,r7
bl      mem_read8
tst     r0,#0x80
and     r4,#0xFF00
orrne   r4,#FLAG_CRY
ands    r0,#0x7F
orreq   r4,#FLAG_ZERO
mov     r1,r0,lsl #1
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

#define sra_rh(vmr, armr) \
    op_sra_##vmr: \
    mov     r0,armr,lsl #16; \
    movs    r0,r0,asr #25; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    ands    r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define sra_rl(vmr, armr) \
    op_sra_##vmr: \
    mov     r0,armr,lsl #24; \
    movs    r0,r0,asr #25; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    ands    r0,#0xFF; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

sra_rh(a, r4)
sra_rh(b, r5)
sra_rl(c, r5)
sra_rh(d, r6)
sra_rl(e, r6)
sra_rh(h, r7)
sra_rl(l, r7)

op_sra__hl:
mov     r0,r7
bl      mem_read8
and     r4,#0xFF00
mov     r1,r0,lsl #24
movs    r1,r1,asr #25
orrcs   r4,#FLAG_CRY
ands    r1,#0xFF
orreq   r4,#FLAG_ZERO
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

#define srl_rh(vmr, armr) \
    op_srl_##vmr: \
    movs    r0,armr,lsr #9; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    ands    r0,#0x7F; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #8; \
    ldmia   r13!,{r15}

#define srl_rl(vmr, armr) \
    op_srl_##vmr: \
    movs    r0,armr,lsr #1; \
    and     r4,#0xFF00; \
    orrcs   r4,#FLAG_CRY; \
    ands    r0,#0x7F; \
    orreq   r4,#FLAG_ZERO; \
    and     armr,#0xFF00; \
    orr     armr,r0; \
    ldmia   r13!,{r15}

srl_rh(a, r4)
srl_rh(b, r5)
srl_rl(c, r5)
srl_rh(d, r6)
srl_rl(e, r6)
srl_rh(h, r7)
srl_rl(l, r7)

op_srl__hl:
mov     r0,r7
bl      mem_read8
tst     r0,#0x01
and     r4,#0xFF00
orrne   r4,#FLAG_CRY
ands    r0,#0xFE
orreq   r4,#FLAG_ZERO
mov     r1,r0,lsr #1
mov     r0,r7
ldmia   r13!,{r14}
b       mem_write8

#define swap_rh(vmr, armr) \
    op_swap_##vmr: \
    and     r0,armr,#0xF000; \
    and     r1,armr,#0x0F00; \
    orrs    r0,r1,r0,lsr #8; \
    and     armr,#0x00FF; \
    orr     armr,r0,lsl #4; \
    and     r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    ldmia   r13!,{r15}

#define swap_rl(vmr, armr) \
    op_swap_##vmr: \
    and     r0,armr,#0xF0; \
    and     r1,armr,#0x0F; \
    orrs    r0,r1,lsl #8; \
    and     armr,#0xFF00; \
    orr     armr,r0,lsr #4; \
    and     r4,#0xFF00; \
    orreq   r4,#FLAG_ZERO; \
    ldmia   r13!,{r15}

op_swap_a:
and     r0,r4,#0xF000
and     r1,r4,#0x0F00
orrs    r0,r1,r0,lsr #8
mov     r4,r0,lsl #4
orreq   r4,#FLAG_ZERO
ldmia   r13!,{r15}

swap_rh(b, r5)
swap_rl(c, r5)
swap_rh(d, r6)
swap_rl(e, r6)
swap_rh(h, r7)
swap_rl(l, r7)

op_swap__hl:
mov     r0,r7
bl      mem_read8
orrs    r0,r0,lsl #8
mov     r1,r0,lsr #4
mov     r0,r7
and     r4,#0xFF00
orreq   r4,#FLAG_ZERO
ldmia   r13!,{r14}
b       mem_write8
