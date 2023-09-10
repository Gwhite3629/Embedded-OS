#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "slab.h"
#include "../stdlib/err.h"

//proc_t* pmalloc(void);

//file_t* fmalloc(void);

/* User facing malloc 
 * The user must specify the type they wish to use so the caches can correctly dispatch objects.
 * The objects that users allocate must be held in an object cache held by the kernel. This is to
 * ensure safety and efficiency. Users can pre-cache their created objects by calling the
 * register functions below.
 */
void* malloc(char *type, uint32_t typesize, uint32_t n);

void* realloc(void *ptr, uint32_t size);

/* User facing free 
 * This returns an object to the appropriate cache.
 */
void free(void *ptr);

void register_constructor(void (*constructor)(void *obj));

void register_destructor(void (*destructor)(void *obj));

#endif // _MALLOC_H_