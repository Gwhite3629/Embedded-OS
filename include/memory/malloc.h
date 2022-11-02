#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "slab.h"
#include "../stdlib/err.h"

//proc_t* pmalloc(void);

//file_t* fmalloc(void);

void* malloc(char *type, uint32_t typesize, uint32_t n);

void* realloc(void *ptr, uint32_t size);

void free(void *ptr);

void register_constructor(void (*constructor)(void *obj));

void register_destructor(void (*destructor)(void *obj));

#endif // _MALLOC_H_