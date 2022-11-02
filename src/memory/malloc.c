#include "../../include/memory/slab.h"
#include "../../include/stdlib/err.h"
#include "../../include/memory/malloc.h"
#include "../../include/stdlib/rbtree.h"
#include "../../include/stdlib/types.h"
#include "../../include/stdlib/lock.h"
#include "../../include/stdlib/list.h"

//proc_t* pmalloc(void);

//file_t* fmalloc(void);

void* malloc(char *type, uint32_t typesize, uint32_t n)
{
    void *addr;
    unsigned long key = hash(type);
    kcache_map *k = NULL;
    rb_node *node;

    // Acquire cache of correct type
    node = rb_find((void *)&key, kcache_tree, &kcache_cmp);
    // If cache doesn't exist, create one
    if (node == NULL) {
        wait_acquire(&slab_lock); // Lock slab allocator
        k = kcache_map_create(type, typesize); // Create new cache
        if (k == NULL) {
            return NULL;
        }
        rb_add(k->loc, kcache_tree, &kcache_less);
        release(&slab_lock); // Release slab allocator
        node = k->loc; // Get rb_node
    // If cache exists get it
    } else {
        k = container_of(node, kcache_map, loc);
    }

    // Take n objects from cache


    cache_alloc(k->c, 0);
}

void* realloc(void *ptr, uint32_t size)
{

}

void free(void *ptr)
{

}