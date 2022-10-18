#ifndef _VMEM_H_
#define _VMEM_H_

#include "mmu.h"
#include "../stdlib/err.h"
#include "../stdlib/types.h"

#define PAGE_SIZE 4096

typedef struct VMEM {
    char *name; // Descriptor
    void *base; // Start Address
    size_t size; // Size
    size_t quantum; // Unit size
    void *(*alloc)(vmem_t *vmp, size_t size, int vflag); // Custom Allocator
    void (*free)(vmem_t *vmp, void *obj, size_t size); // Custom Free-er
    vmem_t *source; // Source arena
    size_t qcache_max; // Max cache size
    int vmflag; // Indicator
} vmem_t;

/**********************************************************
 * VMEM Functions
**********************************************************/

vmem_t *vmem_create(
    char *name,
    void *base,
    size_t size,
    size_t quantum,
    void *(*alloc)(vmem_t *vmp, size_t size, int vflag),
    void (*free)(vmem_t *vmp, void *obj, size_t size),
    vmem_t *source,
    size_t qcache_max,
    int vmflag
);

void vmem_destroy(
    vmem_t *vmp // Arena to destroy
);

void *vmem_alloc(
    vmem_t *vmp, // Source arena
    size_t size, // Size of fetch
    int vmflag // Indicator
);

void vmem_free(
    vmem_t *vmp, // Return arena
    void *addr, // Base address
    size_t size // Size of block
);

void vmem_add(
    vmem_t *vmp, // Arena to expand
    void *addr, // Base address to add
    size_t size, // Size to add
    int vmflag // Indicator
);

#endif // _VMEM_H_