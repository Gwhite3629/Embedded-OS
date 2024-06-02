.equ S_INV_EL1t,      0x0
.equ I_INV_EL1t,      0x1
.equ F_INV_EL1t,      0x2
.equ E_INV_EL1t,      0x3

.equ S_INV_EL1h,      0x4
.equ I_INV_EL1h,      0x5
.equ F_INV_EL1h,      0x6
.equ E_INV_EL1h,      0x7

.equ S_INV_EL0_64,    0x8
.equ I_INV_EL0_64,    0x9
.equ F_INV_EL0_64,    0xA
.equ E_INV_EL0_64,    0xB

.equ S_INV_EL0_32,    0xC
.equ I_INV_EL0_32,    0xD
.equ F_INV_EL0_32,    0xE
.equ E_INV_EL0_32,    0xF

.equ S_FRAME_SIZE,    256

.macro k_entry
    sub sp,     sp,     #S_FRAME_SIZE
    stp x0,     x1,     [sp, #16 * 0]
    stp x2,     x3,     [sp, #16 * 1]
    stp x4,     x5,     [sp, #16 * 2]
    stp x6,     x7,     [sp, #16 * 3]
    stp x8,     x9,     [sp, #16 * 4]
    stp x10,    x11,    [sp, #16 * 5]
    stp x12,    x13,    [sp, #16 * 6]
    stp x14,    x15,    [sp, #16 * 7]
    stp x16,    x17,    [sp, #16 * 8]
    stp x18,    x19,    [sp, #16 * 9]
    stp x20,    x21,    [sp, #16 * 10]
    stp x22,    x23,    [sp, #16 * 11]
    stp x24,    x25,    [sp, #16 * 12]
    stp x26,    x27,    [sp, #16 * 13]
    stp x28,    x29,    [sp, #16 * 14]
    str x30,            [sp, #16 * 15]
.endm

.macro k_exit
    ldp x0,     x1,     [sp, #16 * 0]
    ldp x2,     x3,     [sp, #16 * 1]
    ldp x4,     x5,     [sp, #16 * 2]
    ldp x6,     x7,     [sp, #16 * 3]
    ldp x8,     x9,     [sp, #16 * 4]
    ldp x10,    x11,    [sp, #16 * 5]
    ldp x12,    x13,    [sp, #16 * 6]
    ldp x14,    x15,    [sp, #16 * 7]
    ldp x16,    x17,    [sp, #16 * 8]
    ldp x18,    x19,    [sp, #16 * 9]
    ldp x20,    x21,    [sp, #16 * 10]
    ldp x22,    x23,    [sp, #16 * 11]
    ldp x24,    x25,    [sp, #16 * 12]
    ldp x26,    x27,    [sp, #16 * 13]
    ldp x28,    x29,    [sp, #16 * 14]
    ldr x30,            [sp, #16 * 15]
    add sp,     sp,     #S_FRAME_SIZE
    eret
.endm

.macro handle_invalid err
    k_entry
    mov x0, #\err
    mrs x1, esr_el1
    mrs x2, elr_el1

// Print invlid type here with kprint
    b proc_hang
.endm

.macro ventry label
.align 7
    b \label
.endm

.align 11
.globl vectors
vectors:
    ventry  S_EL1t
    ventry  I_EL1t
    ventry  F_EL1t
    ventry  E_EL1t

    ventry  S_EL1h
    ventry  I_EL1h
    ventry  F_EL1h
    ventry  E_EL1h

    ventry  S_EL0_64
    ventry  I_EL0_64
    ventry  F_EL0_64
    ventry  E_EL0_64

    ventry  S_EL0_32
    ventry  I_EL0_32
    ventry  F_EL0_32
    ventry  E_EL0_32

S_EL1t:
    handle_invalid S_INV_EL1t
#    k_entry
#    bl  swi_handler
#    k_exit

I_EL1t:
    k_entry
    bl  handle_irq
    k_exit

F_EL1t:
    handle_invalid  F_INV_EL1t

E_EL1t:
    handle_invalid  E_INV_EL1t


S_EL1h:
    handle_invalid S_INV_EL1h
#    k_entry
#    bl  swi_handler
#    k_exit

I_EL1h:
    k_entry
    bl  handle_irq
    k_exit

F_EL1h:
    handle_invalid  F_INV_EL1h

E_EL1h:
    handle_invalid  E_INV_EL1h


S_EL0_64:
    handle_invalid S_INV_EL0_64
#    k_entry
#    bl  swi_handler
#    k_exit

I_EL0_64:
    k_entry
    bl  handle_irq
    k_exit

F_EL0_64:
    handle_invalid  F_INV_EL0_64

E_EL0_64:
    handle_invalid  E_INV_EL0_64


S_EL0_32:
    handle_invalid S_INV_EL0_32
#    k_entry
#    bl  swi_handler
#    k_exit

I_EL0_32:
    k_entry
    bl  handle_irq
    k_exit

F_EL0_32:
    handle_invalid  F_INV_EL0_32

E_EL0_32:
    handle_invalid  E_INV_EL0_32


.globl proc_hang
proc_hang: b proc_hang
