#include <memory/mmu_func.h>
//#include <stdlib.h>

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
.equ    SCTLR_I_CACHE_ENABLED,   (1 << 12)
.equ    SCTLR_D_CACHE_DISABLED,  (0 << 2)
.equ    SCTLR_D_CACHE_ENABLED,   (1 << 2)
.equ    SCTLR_MMU_DISABLED,      (0 << 0)
.equ    SCTLR_MMU_ENABLED,       (1 << 0)
.equ    SCTLR_VALUE_MMU_DISABLED, (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)
.equ    SCTLR_VALUE_MMU_ENABLED, (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_ENABLED | SCTLR_D_CACHE_ENABLED | SCTLR_MMU_ENABLED)

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

.equ LOCAL_CONTROL,   0xff800000
.equ LOCAL_PRESCALER, 0xff800008
.equ OSC_FREQ,        54000000
.equ MAIN_STACK,      0x400000

.equ MAX_BASES, 64

// ARM MMU values
.equ MT_DEVICE,       0x0
.equ MT_NORMAL,       0x1
.equ MT_DEVICE_FLAGS, 0x00
.equ MT_NORMAL_FLAGS, 0x44
.equ MAIR_VALUE,      (MT_DEVICE_FLAGS << (8 * MT_DEVICE)) | (MT_NORMAL_FLAGS << (8 * MT_NORMAL))

.equ MM_FLAGS,        (MM_TYPE_BLOCK | (MT_NORMAL << 2) | MM_ACCESS)
.equ MM_DEVICE_FLAGS, (MM_TYPE_BLOCK | (MT_DEVICE << 2) | MM_ACCESS)
.equ MM_PTE_FLAGS,    (MM_TYPE_PAGE  | (MT_NORMAL << 2) | MM_ACCESS | MM_ACCESS_PERM)

.equ TCR_T0SZ,			(64 - 48) 
.equ TCR_T1SZ,			((64 - 48) << 16)
.equ TCR_TG0_4K,			(0 << 14)
.equ TCR_TG1_4K,			(2 << 30)
.equ TCR_VALUE,			(TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K)

.equ EXT_MEM_ADDR, 0x40000000

.equ EXT_MEM_SIZE, 0x40000000

.equ VA_START, 0xffff000000000000

.equ PAGE_MASK, 0xfffffffffffff000

.equ PAGE_SHIFT, 12

.equ TABLE_SHIFT, 9

.equ SECTION_SHIFT, (PAGE_SHIFT + TABLE_SHIFT)

.equ PAGE_SIZE, (1 << PAGE_SHIFT) // 4096 bytes

.equ TABLE_SIZE, (1 << TABLE_SHIFT) // 512 entries

.equ SECTION_SIZE, (1 << SECTION_SHIFT)

.equ PG_DIR_SIZE, (3 * PAGE_SIZE)

.equ LOW_MEMORY, (2 * SECTION_SIZE)
.equ HIGH_MEMORY, IO_BASE
                                    
.equ LVL3_SHIFT, (PAGE_SHIFT + 3 * TABLE_SHIFT)
.equ LVL2_SHIFT, (PAGE_SHIFT + 2 * TABLE_SHIFT)
.equ LVL1_SHIFT, (PAGE_SHIFT + 1 * TABLE_SHIFT)
.equ LVL0_SHIFT, (PAGE_SHIFT + 0 * TABLE_SHIFT)

.equ MM_TYPE_TABLE, 0x3
.equ MM_TYPE_PAGE, 0x3
.equ MM_TYPE_BLOCK, 0x1

.equ MM_ACCESS,       (0x1 << 10)
.equ MM_ACCESS_PERM,  (0x1 << 6)

.equ IO_BASE, 0xfe000000

.equ MEM_SIZE, 0x200000000

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

2:
    mov x0, #0x33ff
    msr cptr_el3, x0
    mov x0, #3 << 20
    //msr cpacr_el1, x0

    ldr     x0, =SCTLR_VALUE_MMU_ENABLED
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
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 5f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    mov     sp, #MAIN_STACK

4:  bl      __create_page_tables

    mov     x0, #VA_START
    add     sp, x0, #LOW_MEMORY

    adrp    x0, pg_dir
    msr     ttbr1_el1, x0

    ldr     x0, =(TCR_VALUE)
    msr     tcr_el1, x0

    ldr     x0, =(MAIR_VALUE)
    msr     mair_el1, x0

5:  bl  kernel_main

    b   2b

.ltorg

    .macro	create_pgd_entry, tbl, virt, tmp1, tmp2
	create_table_entry \tbl, \virt, LVL3_SHIFT, \tmp1, \tmp2
	create_table_entry \tbl, \virt, LVL2_SHIFT, \tmp1, \tmp2
	.endm

	.macro	create_table_entry, tbl, virt, shift, tmp1, tmp2
	lsr	\tmp1, \virt, #\shift
	and	\tmp1, \tmp1, #TABLE_SIZE - 1			// table index
	add	\tmp2, \tbl, #PAGE_SIZE
	orr	\tmp2, \tmp2, #MM_TYPE_TABLE	
	str	\tmp2, [\tbl, \tmp1, lsl #3]
	add	\tbl, \tbl, #PAGE_SIZE					// next level table page
	.endm

	.macro	create_block_map, tbl, phys, start, end, flags, tmp1
	lsr	\start, \start, #SECTION_SHIFT
	and	\start, \start, #TABLE_SIZE - 1			// table index
	lsr	\end, \end, #SECTION_SHIFT
	and	\end, \end, #TABLE_SIZE - 1				// table end index
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

.globl spin_cpu0
spin_cpu0:
    .quad 0

.globl memzero
memzero:
        str     xzr, [x0], #8
        subs    x1, x1, #8
        b.gt    memzero
        ret
