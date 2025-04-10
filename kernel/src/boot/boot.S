#include "boot.h"
#define LOCAL_CONTROL		0xff800000
#define LOCAL_PRESCALER		0xff800008
#define OSC_FREQ            54000000
#define STACK               0x3B100000
.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

_start:
    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // We're not on the main core, so hang in an infinite wait loop
1:  wfe
    b       1b
2:  // We're on the main core!

    mov     x1, #STACK
    mov     sp, x1

    // set up EL1
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    // running at EL3?
    cmp     x0, #12
    bne     3f
    // should never be executed, just for completeness
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, 3f
    msr     elr_el3, x2
    eret

    // running at EL2?
3:  cmp     x0, #4
    beq     4f
    msr     sp_el1, x1
    // enable CNTP for EL1
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr
    // enable AArch64 in EL1
    mov     x0, #(1 << 31)      // AArch64
    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
    msr     hcr_el2, x0
    mrs     x0, hcr_el2
    // disable sp alignment fault
    //mrs x2, sctlr_el1
    //and x2, x2, #0xfffffffffffffff7
    //msr sctlr_el1, x2
    // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 4f
    msr     elr_el2, x2
    eret

4:  
    mov     x1, #STACK
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
5:  cbz     w2, 6f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 5b

    // jump to C code, should not return
6:  bl      main
    // for failsafe, halt this core too
    b       1b
