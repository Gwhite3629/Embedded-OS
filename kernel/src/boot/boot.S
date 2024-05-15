#include "../../include/memory/mmu.h"
#include "../../include/stdlib.h"

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

.section ".text.boot"

.globl _start
_start:
        ldr	pc, reset_addr
	ldr	pc, undefined_instruction_addr
	ldr	pc, software_interrupt_addr
	ldr	pc, prefetch_abort_addr
	ldr	pc, data_abort_addr
	ldr	pc, unused_handler_addr
	ldr	pc, interrupt_addr
	ldr	pc, fast_interrupt_addr
reset_addr:			.word	reset
undefined_instruction_addr:	.word	undef_handler
software_interrupt_addr:	.word	swi_handler
prefetch_abort_addr:		.word	abort_handler
data_abort_addr:		.word	abort_handler
unused_handler_addr:		.word	undef_handler
interrupt_addr:			.word	interrupt_handler
fast_interrupt_addr:		.word	fiq_handler

reset:
        mov     sp, #0x8000
        mov	r3, #(CPSR_MODE_IRQ | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
	mov	sp, #0x4000

        mov	r3, #(CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
        ldr	r3, =_start
	mov	r4, #0x0000
	/* Quick way to copy 256 bytes of memory */
	ldmia	r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
	stmia	r4!,{r5, r6, r7, r8, r9, r10, r11, r12}
	ldmia	r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
	stmia	r4!,{r5, r6, r7, r8, r9, r10, r11, r12}

        mrs     x0, mpidr_el1
        and     x0, x0,#0xFF
        cbz     x0, master
        b       proc_hang

proc_hang:
        b       proc_hang

master:
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

sx

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
