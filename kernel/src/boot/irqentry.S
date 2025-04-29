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


.macro k_entry
    stp     x29, x30, [sp, #-16]!

    mrs     x29, elr_el1
    mrs     x30, spsr_el1
    stp     x29, x30, [sp, #-16]!

    stp     x27, x28, [sp, #-16]!
    stp     x25, x26, [sp, #-16]!
    stp     x23, x24, [sp, #-16]!
    stp     x21, x22, [sp, #-16]!
    stp     x19, x20, [sp, #-16]!
    stp     x17, x18, [sp, #-16]!
    stp     x15, x16, [sp, #-16]!
    stp     x13, x14, [sp, #-16]!
    stp     x11, x12, [sp, #-16]!
    stp     x9, x10, [sp, #-16]!
    stp     x7, x8, [sp, #-16]!
    stp     x5, x6, [sp, #-16]!
    stp     x3, x4, [sp, #-16]!
    stp     x1, x2, [sp, #-16]!
    str         x0, [sp, #-16]!
    isb
.endm

.macro k_exit
    dsb sy

    ldr         x0, [sp], #16
    ldp	    x1, x2, [sp], #16
    ldp     x3, x4, [sp], #16
    ldp     x5, x6, [sp], #16
    ldp     x7, x8, [sp], #16
    ldp     x9, x10, [sp], #16
    ldp     x11, x12, [sp], #16
    ldp     x13, x14, [sp], #16
    ldp     x15, x16, [sp], #16
    ldp     x17, x18, [sp], #16
    ldp     x19, x20, [sp], #16
    ldp     x21, x22, [sp], #16
    ldp     x23, x24, [sp], #16
    ldp     x25, x26, [sp], #16
    ldp     x27, x28, [sp], #16

    ldp     x29, x30, [sp], #16
    msr     elr_el1, x29
    msr     spsr_el1, x30

    ldp     x29, x30, [sp], #16

    dsb nsh
    eret
.endm

.macro handle_skip err
    isb
    eret
.endm

.macro handle_invalid err
    k_entry
    mov x0, #\err
    bl  handle_error
    k_exit
.endm

.macro ventry label
.align 7
    b \label
.endm

.align 11
.globl vectors
vectors:
    ventry S_EL1t
    ventry I_EL1t
    ventry F_EL1t
    ventry E_EL1t

    ventry S_EL1h
    ventry I_EL1h
    ventry F_EL1h
    ventry E_EL1h
    
    ventry S_EL0_64
    ventry I_EL0_64
    ventry F_EL0_64
    ventry E_EL0_64
    
    ventry S_EL0_32
    ventry I_EL0_32
    ventry F_EL0_32
    ventry E_EL0_32

S_EL1t:
    handle_invalid S_INV_EL1t

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

I_EL0_32:
    k_entry
    bl  handle_irq
    k_exit

F_EL0_32:
    handle_invalid  F_INV_EL0_32

E_EL0_32:
    handle_invalid  E_INV_EL0_32
