#include <memory/vmem.h>
#include <memory/slab.h>
#include <stdlib.h>

kcache_t *kcache;

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
)
{
    kcache_t *cache = NULL;
    err_t ret = E_NOERR;

    cache = kcache_alloc(size, align, 0);

}

static kcache_t *kcache_alloc(size_t size, uint32_t align, int32_t align_offset)
{
    kcache_t *k = NULL;
    slab_t *sp;

    if (size < PAGE_SIZE) {
        
    } else {

    }

    return k;
}

void *cache_alloc(
    kcache_t *cptr, // Cache to retrieve from
    int flags // Indicator
)
{

}

void cache_free(
    kcache_t *cptr, // Cache to return to
    void *buf // Object to return
)
{

}

void cache_destroy(
    kcache_t *cptr // Return objects to arena
)
{

}