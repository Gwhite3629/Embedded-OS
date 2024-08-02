//#include "../memory/mmu_func.h"

#define LOCAL_CONTROL   0xff800000
#define LOCAL_PRESCALER 0xff800008
#define OSC_FREQ        54000000
#define MAIN_STACK      0x00400000

/* CPSR */
#define CPSR_MODE_USER		        0x10
#define CPSR_MODE_FIQ		        0x11
#define CPSR_MODE_IRQ		        0x12
#define CPSR_MODE_SVC		        0x13
#define CPSR_MODE_ABORT	        0x17
#define CPSR_MODE_UNDEFINED	        0x1b
#define CPSR_MODE_SYSTEM	        0x1f
#define CPSR_MODE_FIQ_DISABLE	        (1 << 6)// F set, FIQ disabled
#define CPSR_MODE_IRQ_DISABLE	        (1 << 7)// I set, IRQ disabled
#define CPSR_MODE_ABORT_DISABLE        (1 << 8)// A set, ABT disabled

/* SCTLR_EL1, system control register */
#define SCTLR_RESERVED          (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN  (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN (0 << 24)
#define SCTLR_I_CACHE_DISABLED  (0 << 12)
#define SCTLR_I_CACHE_ENABLED   (1 << 12)
#define SCTLR_D_CACHE_DISABLED  (0 << 2)
#define SCTLR_D_CACHE_ENABLED   (1 << 2)
#define SCTLR_MMU_DISABLED      (0 << 0)
#define SCTLR_MMU_ENABLED       (1 << 0)
#define SCTLR_VALUE_MMU_DISABLED (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)
#define SCTLR_VALUE_MMU_ENABLED (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_ENABLED | SCTLR_D_CACHE_ENABLED | SCTLR_MMU_ENABLED)

/* HCR_EL2, hypervisor config register */
#define HCR_RW                     (1 << 31)
#define HCR_VALUE                  HCR_RW

/* SCR_EL3, secure config register */
#define SCR_RESERVED               (3 << 4)
#define SCR_RW                     (1 << 10)
#define SCR_NS                     (1 << 0)
#define SCR_VALUE                  (SCR_RESERVED | SCR_RW | SCR_NS)

/* SPSR */
#define SPSR_MASK_ALL              (7 << 6)
#define SPSR_EL1h                  (5 << 0)
#define SPSR_VALUE                 (SPSR_MASK_ALL | SPSR_EL1h)

/* ESR_EL1, exception syndrome register */
#define ESR_ELx_EC_SHIFT           26
#define ESR_ELx_EC_SVC64           0x15
#define ESR_ELx_EC_DABT_LOW        0x24

#define MAX_BASES 64

// ARM MMU values
//#define MT_DEVICE       0x0
//#define MT_NORMAL       0x1
//#define MT_DEVICE_FLAGS 0x00
//#define MT_NORMAL_FLAGS 0x44
//#define MAIR_VALUE      (MT_DEVICE_FLAGS << (8 * MT_DEVICE)) | (MT_NORMAL_FLAGS << (8 * MT_NORMAL))

//#define MM_FLAGS        (MM_TYPE_BLOCK | (MT_NORMAL << 2) | MM_ACCESS)
//#define MM_DEVICE_FLAGS (MM_TYPE_BLOCK | (MT_DEVICE << 2) | MM_ACCESS)
//#define MM_PTE_FLAGS    (MM_TYPE_PAGE  | (MT_NORMAL << 2) | MM_ACCESS | MM_ACCESS_PERM)

#define TCR_T0SZ			(64 - 48) 
#define TCR_T1SZ			((64 - 48) << 16)
#define TCR_TG0_4K			(0 << 14)
#define TCR_TG1_4K			(2 << 30)
#define TCR_VALUE			(TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K)

#define EXT_MEM_ADDR 0x40000000

#define EXT_MEM_SIZE 0x40000000

#define VA_START 0xffff000000000000

#define PAGE_MASK 0xfffffffffffff000

#define PAGE_SHIFT 12

#define TABLE_SHIFT 9

#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT) // 4096 bytes

#define TABLE_SIZE (1 << TABLE_SHIFT) // 512 entries

#define SECTION_SIZE (1 << SECTION_SHIFT)

#define PG_DIR_SIZE (3 * PAGE_SIZE)

#define LOW_MEMORY (2 * SECTION_SIZE)
#define HIGH_MEMORY IO_BASE
                                    
#define LVL3_SHIFT (PAGE_SHIFT + 3 * TABLE_SHIFT)
#define LVL2_SHIFT (PAGE_SHIFT + 2 * TABLE_SHIFT)
#define LVL1_SHIFT (PAGE_SHIFT + 1 * TABLE_SHIFT)
#define LVL0_SHIFT (PAGE_SHIFT + 0 * TABLE_SHIFT)

#define MM_TYPE_TABLE 0x3
#define MM_TYPE_PAGE 0x3
#define MM_TYPE_BLOCK 0x1

#define MM_ACCESS       (0x1 << 10)
#define MM_ACCESS_PERM  (0x1 << 6)

#define IO_BASE 0xfe000000

#define MEM_SIZE 0x200000000
