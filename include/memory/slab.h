#ifndef _SLAB_H_
#define _SLAB_H_

#include "vmem.h"
#include "../stdlib/err.h"
#include "../stdlib/types.h"

extern lock_t slab_lock;

typedef struct KMEM_CACHE {
    char *name;
    DLL full;
    DLL partial;
    DLL free;
    uint32_t color;
    size_t size;
    lock_t lock;
    void (*constructor)(void *obj, size_t size);
    void (*destructor)(void *obj, size_t size);
    vmem_t *vmp;
} kcache_t;

typedef struct SLAB {
    DLL list;
    kcache_t *cache;
    void *start;
    void *free;
    uint32_t refcount;
} slab_t;

typedef struct BUFCTL {
    bufctl_t *next;
} bufctl_t;

/**********************************************************
 * Cache Functions
**********************************************************/
kcache_t *cache_create(
    char *name, // Descriptor
    size_t size,
    uint32_t align, // Optional alignment (0 default)
    err_t (*construct)(void *obj, void *private, int kflag), // Custom constructor
    void (*destruct)(void *obj, void *private),  // Custom destructor
    void (*reclaim)(void *private),
    void *private,
    vmem_t *vmp,
    int cflag
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