#ifndef _MEMSET_H_
#define _MEMSET_H_

#ifndef __ASSEMBLER__
#include "types.h"
#endif // __ASSEMBLER__
#define memzero(s,n) memset((s), 0, (n))
void *memset(void *dest, char val, size_t n);

#endif // _MEMSET_H_