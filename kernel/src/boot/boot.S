#include "boot.h"
#define LOCAL_CONTROL		0xff800000
#define LOCAL_PRESCALER		0xff800008
#define OSC_FREQ            54000000
#define STACK               0x3B100000

#define GIC_DISTB   0xff841000
#define GIC_CPUB    0xff842000

#define GICC_CTRLR      0x0
#define GICC_PMR        0x4
#define IT_NR           0x8
#define GICD_CTRLR      0x0
#define GICD_IGROUPR    0x80

.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

_start:
// Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // We're not on the main core, so hang in an infinite wait loop
    adr     x5, spin_cpu0
1:  wfe
    ldr     x4, [x5, x1, lsl #3]
    cbz     x4, 1b
2:  // We're on the main core!

//    bl setup_timer

    bl setup_gic

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

.ltorg

.org 0x110
.globl spin_cpu0
spin_cpu0:
    .quad 0

setup_timer:
    ldr     x0, =LOCAL_CONTROL
    str     wzr, [x0]
    mov     w1, 0x80000000
    str     w1, [x0, #(LOCAL_PRESCALER - LOCAL_CONTROL)]

    ldr     x0, =OSC_FREQ
    msr     cntfrq_el0, x0
    msr     cntvoff_el2, xzr



setup_gic:
    mrs x0, MPIDR_EL1
    ldr x2, =GIC_DISTB
    tst x0, #0x3
    b.eq 2f

    mov w0, #3
    str w0, [x2, #GICD_CTRLR]
2:
    add x1, x2, #(GIC_CPUB - GIC_DISTB)
    mov w0, #0x1e7
    str w0, [x1, #GICC_CTRLR]
    mov w0, #0xff
    str w0, [x1, #GICC_PMR]
    add x2, x2, #GICD_IGROUPR
    mov x0, #(IT_NR * 4)
    mov w1, #~0
3:
    subs x0, x0, #4
    str w1, [x2, x0]
    b.ne 3b
    ret

.ltorg




