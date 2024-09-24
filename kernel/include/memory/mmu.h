#ifndef _MMU_H_
#define _MMU_H_

#include "../process/proc.h"
#include "../stdlib.h"
#include "../drivers/graphics/framebuffer.h"

//void free_page(unsigned long page);
//void map_page(proc_t *p, unsigned long va, unsigned long page);
//int copy_virtual(proc_t *dst);
//unsigned long get_kernel_page();
//unsigned long get_user_page(proc_t *p, unsigned long va);

#define PT_SIZE (1 << 21) // 2MB
#define PT_NUM  512
#define TLB_ALIGN   4096

#define MT_DEVICE_NGNRNE    0x0
#define MT_DEVICE_NGNRE     0x1
#define MT_DEVICE_GRE       0x2
#define MT_NORMAL_NC        0x3
#define MT_NORMAL           0x4

void identity_map(void);
uint64_t virtualmap(uint64_t phys_addr, uint8_t memattrs);
void enable_MMU(void);
void enable_mmu_tables (void* map1to1, void* virtualmap);

#endif // _MMU_H_
