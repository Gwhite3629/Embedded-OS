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
    sub sp, sp, #192
    stp	x0, x1, [sp, #0]
	stp	x2, x3, [sp, #16]
	stp	x4, x5, [sp, #32]
	stp	x6, x7, [sp, #48]
	stp	x8, x9, [sp, #64]
	stp	x10, x11, [sp, #80]
	stp	x12, x13, [sp, #96]
	stp	x14, x15, [sp, #112]
	stp	x16, x17, [sp, #128]
	stp	x18, x29, [sp, #144]
    stp x30, xzr, [sp, #160]
.endm

.macro k_exit
    ldp	x0, x1, [sp, #0]
	ldp	x2, x3, [sp, #16]
	ldp	x4, x5, [sp, #32]
	ldp	x6, x7, [sp, #48]
	ldp	x8, x9, [sp, #64]
	ldp	x10, x11, [sp, #80]
	ldp	x12, x13, [sp, #96]
	ldp	x14, x15, [sp, #112]
	ldp	x16, x17, [sp, #128]
	ldp	x18, x29, [sp, #144]
    ldp x30, xzr, [sp, #160]
    add sp, sp, #192
    eret
    eret
.endm

.macro handle_invalid err
    k_entry
    mov x0, #\err
    bl  handle_error
    k_exit
.endm

.globl vectors
.balign 2048
vectors:
    b S_EL1t
.balign 0x80   
    b I_EL1t
.balign 0x80
    b F_EL1t
.balign 0x80
    b E_EL1t
.balign 0x80
    b S_EL1h
.balign 0x80
    b I_EL1h
.balign 0x80
    b F_EL1h
.balign 0x80
    b E_EL1h
.balign 0x80
    b S_EL0_64
.balign 0x80
    b I_EL0_64
.balign 0x80
    b F_EL0_64
.balign 0x80
    b E_EL0_64
.balign 0x80
    b S_EL0_32
.balign 0x80
    b I_EL0_32
.balign 0x80
    b F_EL0_32
.balign 0x80
    b E_EL0_32

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
