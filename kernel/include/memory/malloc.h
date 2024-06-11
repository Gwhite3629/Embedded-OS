#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "../stdlib.h"

#define UNALLOCATED 0
#define RESERVED 1
#define CULLED 2
#define AVAILABLE 3

extern const char *stat_names[12];

typedef struct __attribute__((packed)) {
    int size;           // Size of alloced space
    int flag;           // Pointer status (add perms)
    void *base;         // Base region address
    void *addr;         // Address of alloced space
} smart_ptr;

typedef struct __attribute__((packed)) {
    int alloc_size;     // Total region size (2MB)
    int used_size;      // Total alloced space
    int n_chunks;       // Number of pointers
    void *base_addr;    // Base region address
    smart_ptr **chunks; // List of smart_ptr locations
} region_t;

typedef struct __attribute__((packed)) {
    int alignment;      // 2MB (value not used)
    int n_regions;      // Number of 2MB user regions
    region_t **regions; // List of region locations
} heap_t;

extern heap_t *kernel_heap;
extern int ret;

#define ALIGN 4096

#define INIT_SIZE (2 >> (9 + 12)) // 512 * 4096 bytes, 1 whole PTE about 2MB

#define CHUNK_INFO_SIZE sizeof(smart_ptr)
#define REGION_INFO_SIZE sizeof(region_t)
#define HEAP_INFO_SIZE sizeof(heap_t)

#define REGION_ARR sizeof(region_t *)
#define CHUNK_ARR sizeof(smart_ptr *)

#define HEAP_BASE_OFFSET (REGION_INFO_SIZE + (2*CHUNK_INFO_SIZE))
#define NEW_REGION_SIZE (REGION_INFO_SIZE + 3*CHUNK_INFO_SIZE + 3*CHUNK_ARR)

#define F_CHECK(F_CHUNK) \
    ((F_CHUNK->flag & 1) \
    ^ (F_CHUNK->flag & 2))

#define new(ptr, size, type) \
    ptr = (type *)alloc(global_heap, size*sizeof(type)); \
    VALID(ptr, E_NOMEM);

#define alt(ptr, size, type) \
    ptr = (type *)change(global_heap, ptr, size*sizeof(type)); \
    VALID(ptr, E_NOMEM);

#define del(ptr) \
    cull(global_heap, ptr); \
    VALID(global_heap, E_NOMEM);

heap_t *create(int alignment, int size);

heap_t *grow_kheap(heap_t *h);

void create_region(heap_t *h, int size);

void destroy(heap_t *h);

void *alloc(heap_t *h, int n);

void cull(heap_t *h, void *ptr);

void clean(heap_t *h);

void *change(heap_t *h, void *ptr, int size);

extern heap_t *global_heap;

#endif // _MALLOC_H_
