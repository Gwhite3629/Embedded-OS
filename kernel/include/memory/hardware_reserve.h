#ifndef _HARDWARE_RESERVE_H_
#define _HARDWARE_RESERVE_H_

#include "../stdlib/types.h"

#define MEM_K 0 // Kernel memory
#define MEM_U 1 // User memory

void memory_init(unsigned long mem_kernel);

unsigned long reserve(uint32_t n_chunks, uint32_t type);
int32_t relinquish(unsigned long start, uint32_t n_chunks);
int32_t sum_free(void);
uint32_t mem_total(void);

#endif // _HARDWARE_RESERVE_H_