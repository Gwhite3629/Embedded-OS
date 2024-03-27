#ifndef _SLAB_H_
#define _SLAB_H_

#include "vmem.h"
#include "../stdlib/err.h"
#include "../stdlib/types.h"
#include "../stdlib/container_of.h"
#include "../stdlib/rbtree.h"
#include "../stdlib/lock.h"

extern lock_t slab_lock;

typedef struct KCACHE_MAP {
    kcache_t *c;
    char *name;
    rb_node *loc;
} kcache_map;

static inline int
kcache_cmp(const void *key, const struct rb_node *node)
{
	long k = *(long *)key;
    kcache_map *m;
	m = container_of(node, kcache_map, loc);
    char *name = m->name;
    long l = (long)hash(name);
    return k - l;
}

static inline bool
kcache_less(rb_node *l, const rb_node *r)
{
    kcache_map *lm;
    kcache_map *rm;
    lm = container_of(l, kcache_map, loc);
    rm = container_of(r, kcache_map, loc);
    char *lname = lm->name;
    char *rname = rm->name;
    long lk = (long)hash(lname);
    long rk = (long)hash(rname);
    return lk < rk;
}

extern rb_root *kcache_tree;
extern kcache_map *kcache_root;

typedef struct KMEM_CACHE {
    char *name;
    DLL *full;
    DLL *partial;
    DLL *free;
    uint32_t color;
    size_t size;
    lock_t lock;
    void (*constructor)(void *obj, size_t size);
    void (*destructor)(void *obj, size_t size);
    vmem_t *vmp;
} kcache_t;

typedef struct SLAB {
    DLL *list;
    kcache_t *cache;
    void *start;
    void *free;
    uint32_t refcount;
} slab_t;

typedef struct BUFCTL {
    bufctl_t *next;
    char color:2;   // Determines what the purpose of the BUFCTL is.
                    // 00 = INVALID
                    // 01 = Span
                    // 10 = Allocated segment
                    // 11 = Free segment
} bufctl_t;

static inline kcache_map *kcache_map_create(char *name, size_t size) {

};

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