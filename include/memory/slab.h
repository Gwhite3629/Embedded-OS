#ifndef _SLAB_H_
#define _SLAB_H_

#include "vmem.h"
#include "../stdlib/err.h"

typedef struct KMEM_CACHE {
    DLL full;
    DLL partial;
    DLL free;
    size_t size;
    uint32_t flags;
    uint32_t num;
    lock_t lock;
} kcache_t;

typedef struct SLAB {
    DLL list;
    uint32_t color;
    void *start;
    uint32_t used;
    bufctl_t free;
} slab_t;

typedef uint32_t bufctl_t;

#define slab_bufctl(slabptr) \
    ((bufctl_t *)(((slab_t *)slabptr) + 1))

/**********************************************************
 * Cache Functions
**********************************************************/
kcache_t *cache_create(
    char *name, // Descriptor
    uint32_t align, // Optional alignment (0 default)
    void (*construct)(void *, size_t), // Custom constructor
    void (*destruct)(void *, size_t)   // Custom destructor
);

void *cache_alloc(
    kcache_t *cptr, // Cache to retrieve from
    int flags // Indicator
);

void cache_free(
    kcache_t *cptr, // Cache to return to
    void *buf // Object to return
);

void cache_destroy(
    kcache_t *cptr // Return objects to arena
);

/**********************************************************
 * 
**********************************************************/

#endif // _SLAB_H_