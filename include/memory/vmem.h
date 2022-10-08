#ifndef _VMEM_H_
#define _VMEM_H_

#include "mmu.h"
#include "../stdlib/err.h"

typedef struct VMEM {
    char *name, // Descriptor
    void *base, // Start Address
    size_t size, // Size
    size_t quantum, // Unit size
    void *(*alloc)(vmem_t *, size_t, int), // Custom Allocator
    void (*free)(vmem_t *, void *, size_t), // Custom Free-er
    vmem_t *source, // Source arena
    size_t qcache_max, // Max cache size
    int vmflag // Indicator
} vmem_t;

/**********************************************************
 * VMEM Functions
**********************************************************/

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