#ifndef _MMU_FUNC_H_
#define _MMU_FUNC_H_

#include "../stdlib.h"

void tlb_invalidate_all(void);
void icache_invalidate_all(void);
void disable_l1_dcache(void);
void invalidate_l1_dcache(void);
void enable_l1_dcache(void);
void flush_icache(void);
void flush_dcache(uint32_t start_addr, uint32_t end_addr);


#endif // _MMU_FUNC_H_