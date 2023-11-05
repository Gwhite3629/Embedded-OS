#ifndef _MMU_H_
#define _MMU_H_

#include "../stdlib/err.h"
#include "../process/proc.h"
#include "../stdlib.h"

// ARM MMU values
#define MT_DEVICE       0x0
#define MT_NORMAL       0x1
#define MT_DEVICE_FLAGS 0x00
#define MT_NORMAL_FLAGS 0x44
#define MAIR_VALUE      (MT_DEVICE_FLAGS << (8 * MT_DEVICE)) | (MT_NORMAL_FLAGS << (8 * MT_NORMAL))

#define MM_FLAGS        (MM_TYPE_BLOCK | (MT_NORMAL << 2) | MM_ACCESS)
#define MM_DEVICE_FLAGS (MM_TYPE_BLOCK | (MT_DEVICE << 2) | MM_ACCESS)
#define MM_PTE_FLAGS    (MM_TYPE_PAGE  | (MT_NORMAL << 2) | MM_ACCESS | MM_ACCESS_PERM)

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

typedef struct mmu_data {
    unsigned long LVL3;
    int n_user_pages;
    int n_kernel_pages;
} MM_t;

void free_page(unsigned long page);
void map_page(proc_t *p, unsigned long va, unsigned long page);
int copy_virtual(proc_t *dst);
unsigned long get_kernel_page();
unsigned long get_user_page(proc_t *p, unsigned long va);

#endif // _MMU_H_
