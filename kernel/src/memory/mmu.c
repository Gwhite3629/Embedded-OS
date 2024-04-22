#include <stdlib/err.h>
#include <memory/hardware_reserve.h>
#include <process/proc.h>
#include <memory/mmu.h>

unsigned long map_table(unsigned long *table, unsigned long shift, unsigned long va, int *new_table)
{
    unsigned long index = va >> shift;
    index = index & (TABLE_SIZE - 1);
    if (!table[index]) {
        *new_table = 1;
        unsigned long next_level_table = reserve(1, MEM_K);
        unsigned long entry = next_level_table | MM_TYPE_TABLE;
        table[index] = entry;
        return next_level_table;
    } else {
        *new_table = 0;
    }
    return table[index] & PAGE_MASK;
}

void map_table_entry(unsigned long *LVL0, unsigned long va, unsigned long pa)
{
    unsigned long index = va >> PAGE_SHIFT;
    index = index & (TABLE_SIZE - 1);
    unsigned long entry = pa | MM_PTE_FLAGS;
    LVL0[index] = entry;
}

void map_page(proc_t *p, unsigned long va, unsigned long page)
{
    unsigned long LVL3;
    if (!p->mm.LVL3) {
        p->mm.LVL3 = get_kernel_page();
        p->mm.n_kernel_pages++;
    }
    LVL3 = p->mm.LVL3;
    int new_table;
    unsigned long LVL2 = map_table((unsigned long *)(LVL3 + VA_START), LVL3_SHIFT, va, &new_table);
    if (new_table) {
        p->mm.n_kernel_pages++;
    }
    unsigned long LVL1 = map_table((unsigned long *)(LVL2 + VA_START), LVL2_SHIFT, va, &new_table);
    if (new_table) {
        p->mm.n_kernel_pages++;
    }
    unsigned long LVL0 = map_table((unsigned long *)(LVL1 + VA_START), LVL1_SHIFT, va, &new_table);
    if (new_table) {
        p->mm.n_kernel_pages++;
    }
    map_table_entry((unsigned long *)(LVL0 + VA_START), va, page);
    p->mm.n_user_pages++;
}

unsigned long get_kernel_page()
{
    unsigned long page = reserve(1, MEM_K);
    if (page == 0) {
        return 0;
    }
    return page + VA_START;
}

unsigned long get_user_page(proc_t *p, unsigned long va)
{
    unsigned long page = reserve(1, MEM_U);
    if (page == 0) {
        return 0;
    }
    map_page(p, va, page);
    return page + VA_START;
}

void free_page(unsigned long page)
{
    int32_t r = relinquish(page, 1);
    if (r != 0) {
        return;
    }
}