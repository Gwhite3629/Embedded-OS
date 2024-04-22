#ifndef _MMU_H_
#define _MMU_H_

#include "../process/proc.h"
#include "../stdlib.h"

void free_page(unsigned long page);
void map_page(proc_t *p, unsigned long va, unsigned long page);
int copy_virtual(proc_t *dst);
unsigned long get_kernel_page();
unsigned long get_user_page(proc_t *p, unsigned long va);

#endif // _MMU_H_
