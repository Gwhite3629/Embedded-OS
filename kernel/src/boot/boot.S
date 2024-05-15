#include <memory/mmu_func.h>
#include <stdlib.h>

/* System definitions, from ARM ARM */

/* CPSR */
.equ	CPSR_MODE_USER,		        0x10
.equ	CPSR_MODE_FIQ,		        0x11
.equ	CPSR_MODE_IRQ,		        0x12
.equ	CPSR_MODE_SVC,		        0x13
.equ	CPSR_MODE_ABORT,	        0x17
.equ	CPSR_MODE_UNDEFINED,	        0x1b
.equ	CPSR_MODE_SYSTEM,	        0x1f
.equ	CPSR_MODE_FIQ_DISABLE,	        (1 << 6)// F set, FIQ disabled
.equ	CPSR_MODE_IRQ_DISABLE,	        (1 << 7)// I set, IRQ disabled
.equ	CPSR_MODE_ABORT_DISABLE,        (1 << 8)// A set, ABT disabled

/* SCTLR_EL1, system control register */
.equ    SCTLR_RESERVED,          (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
.equ    SCTLR_EE_LITTLE_ENDIAN,  (0 << 25)
.equ    SCTLR_EOE_LITTLE_ENDIAN, (0 << 24)
.equ    SCTLR_I_CACHE_DISABLED,  (0 << 12)
.equ    SCTLR_D_CACHE_DISABLED,  (0 << 2)
.equ    SCTLR_MMU_DISABLED,      (0 << 0)
.equ    SCTLR_MMU_ENABLED,       (1 << 0)
.equ    SCTLR_VALUE_MMU_DISABLED, (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

/* HCR_EL2, hypervisor config register */
.equ HCR_RW,                     (1 << 31)
.equ HCR_VALUE,                  HCR_RW

/* SCR_EL3, secure config register */
.equ SCR_RESERVED,               (3 << 4)
.equ SCR_RW,                     (1 << 10)
.equ SCR_NS,                     (1 << 0)
.equ SCR_VALUE,                  (SCR_RESERVED | SCR_RW | SCR_NS)

/* SPSR */
.equ SPSR_MASK_ALL,              (7 << 6)
.equ SPSR_EL1h,                  (5 << 0)
.equ SPSR_VALUE,                 (SPSR_MASK_ALL | SPSR_EL1h)

/* ESR_EL1, exception syndrome register */
.equ ESR_ELx_EC_SHIFT,           26
.equ ESR_ELx_EC_SVC64,           0x15
.equ ESR_ELx_EC_DABT_LOW,        0x24

#define LOCAL_CONTROL   0xff800000
#define LOCAL_PRESCALER 0xff800008
#define OSC_FREQ        54000000
#define MAIN_STACK      0x400000

.section ".text.boot"

.globl _start
_start:
    ldr x0, =LOCAL_CONTROL
    str wzr, [x0]
    mov w1, 0x80000000
    str w1, [x0, #(LOCAL_PRESCALER - LOCAL_CONTROL)]

    ldr x0, =OSC_FREQ
    msr cntfrq_el0, x0
    msr cntvoff_el2, xzr

    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, 2f

    adr x5, spin_cpu0

1:
    wfe
    ldr x4, [x5, x1, lsl #3]
    cbz x4, 1b

    ldr x2, =__stack_start
    lsl x1, x1, #9
    add x3, x2, x1
    mov sp, x3

    mov x0, #0
    mov x1, #0
    mov x2, #0
    mov x3, #0
    br x4
    b 1b

2:
    mov x0, #0x33ff
    msr cptr_el3, x0
    mov x0, #3 << 20
    msr cpacr_el1, x0



    ldr     x0, =SCTLR_VALUE_MMU_DISABLED
    msr     sctlr_el1, x0

    ldr     x0, =HCR_VALUE
    msr     hcr_el2, x0

    ldr     x0, =SCR_VALUE
    msr     scr_el3, x0

    ldr     x0, =SPSR_VALUE
    msr     spsr_el3, x0

    adr     x0, el1_entry
    msr     elr_el3, x0

    eret

el1_entry:
    adr     x0, bss_begin
    adr     x1, bss_end
    sub     x1, x1, x0
    bl      memzero

    bl      __create_page_tables

    mov     x0, #VA_START
    add     sp, x0, #LOW_MEMORY

    adrp    x0, pg_dir
    msr     ttbr1_el1, x0

    ldr     x0, =(TCR_VALUE)
    msr     tcr_el1, x0

    ldr     x0, =(MAIR_VALUE)
    msr     mair_el1, x0

    ldr     x2, =kernel_main

    mov     x0, #SCTLR_MMU_ENABLED
    msr     sctlr_el1, x0

    br x2



    .macro	create_pgd_entry, tbl, virt, tmp1, tmp2
	create_table_entry \tbl, \virt, PGD_SHIFT, \tmp1, \tmp2
	create_table_entry \tbl, \virt, PUD_SHIFT, \tmp1, \tmp2
	.endm

	.macro	create_table_entry, tbl, virt, shift, tmp1, tmp2
	lsr	\tmp1, \virt, #\shift
	and	\tmp1, \tmp1, #PTRS_PER_TABLE - 1			// table index
	add	\tmp2, \tbl, #PAGE_SIZE
	orr	\tmp2, \tmp2, #MM_TYPE_PAGE_TABLE	
	str	\tmp2, [\tbl, \tmp1, lsl #3]
	add	\tbl, \tbl, #PAGE_SIZE					// next level table page
	.endm

	.macro	create_block_map, tbl, phys, start, end, flags, tmp1
	lsr	\start, \start, #SECTION_SHIFT
	and	\start, \start, #PTRS_PER_TABLE - 1			// table index
	lsr	\end, \end, #SECTION_SHIFT
	and	\end, \end, #PTRS_PER_TABLE - 1				// table end index
	lsr	\phys, \phys, #SECTION_SHIFT
	mov	\tmp1, #\flags
	orr	\phys, \tmp1, \phys, lsl #SECTION_SHIFT			// table entry
9999:	str	\phys, [\tbl, \start, lsl #3]				// store the entry
	add	\start, \start, #1					// next entry
	add	\phys, \phys, #SECTION_SIZE				// next block
	cmp	\start, \end
	b.ls	9999b
	.endm

__create_page_tables:
    mov     x29, x30

    adrp    x0, pg_dir
    mov     x1, #PG_DIR_SIZE
    bl memzero

    adrp    x0, pg_dir
    mov     x1, #VA_START
    create_pgd_entry x0, x1, x2, x3

    mov     x1, xzr
    mov     x2, #VA_START
    ldr     x3, =(VA_START + IO_BASE - SECTION_SIZE)
    create_block_map x0, x1, x2, x3, MM_FLAGS, x4

    mov     x1, #IO_BASE
    ldr     x2, =(VA_START + IO_BASE)
    ldr     x3, =(VA_START + MEM_SIZE - SECTION_SIZE)
    create_block_map x0, x1, x2, x3, MM_DEVICE_FLAGS, x4

    mov x30, x29
    ret

.ltorg

.org 0x110
.globl spin_cpu0
spin_cpu0:
    .quad 0


