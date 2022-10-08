#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "slab.h"
#include "../stdlib/err.h"

proc_t* pmalloc(void);

file_t* fmalloc(void);

void* malloc(uint32_t size);

void* realloc(void *ptr, uint32_t size);

void free(void *ptr);

#endif // _MALLOC_H_