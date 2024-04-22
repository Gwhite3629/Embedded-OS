#include <stdlib.h>
#include <memory/hardware_reserve.h>

#define MAX_MEMORY (1024*1024*1024)     // 1GB
#define KERNEL_RESERVED (16*1024*1024)  // 16MB

#define CHUNK_SIZE 4096

static lock_t MEM_LOCK = {UNLOCKED, KERNEL_OWNER};

static uint32_t MEMORY_TOTAL;

static unsigned long MEMORY_MAP[MAX_MEMORY/CHUNK_SIZE/32];

static uint32_t MAX_CHUNK = 0;

#define mark_used(chunk) \
    set_bit((MEMORY_MAP), chunk)

#define mark_free(chunk) \
    clear_bit((MEMORY_MAP), (chunk))

#define test_used(chunk) \
    get_bit((MEMORY_MAP), (chunk))

static int init(unsigned long memory_total,
                unsigned long memory_kernel) {
    int i;

    if (memory_total > MEMORY_TOTAL) {
        printk("Too much memory\n");
        return -E_NOMEM;
    }

    if (memory_kernel>KERNEL_RESERVED) {
        printk("Kernel too big\n");
        return -E_NOMEM;
    }

    MAX_CHUNK = (memory_total/CHUNK_SIZE);

    for (i = 0; i < (memory_kernel/CHUNK_SIZE); i++) {
        MEMORY_MAP[i] = 0;
    }

    for (i = 0; i < (memory_kernel/CHUNK_SIZE); i++) {
        mark_used(i);
    }

    return 0;
}

static unsigned long find_free(int n_chunks, unsigned long start, unsigned long end) {
    unsigned long i, j;

    for (i=start; i<end; i++) {
        if (!test_used(i)) {
            for(j=0;j<n_chunks;j++) {
                if (test_used((i+j))) break;
            }
            if ((j==n_chunks) && (i+j<end)) {
                return i;
            }
        }
    }

    return -1;
}

int32_t sum_free(void) {
    int32_t total_free=0,i;

    for (i=0; i<MAX_CHUNK; i++) {
        if (!test_used(i)) total_free++;
    }

    return total_free*CHUNK_SIZE;
}

uint32_t mem_total(void)
{
    return MEMORY_TOTAL;
}

unsigned long reserve(uint32_t n_chunks, uint32_t type)
{
    unsigned long first_chunk;
    unsigned long i;
    unsigned long start,end;

    if(n_chunks==0) n_chunks=1;

    if (type == MEM_K) {
        start = 0;
        end = KERNEL_RESERVED/CHUNK_SIZE;
    } else if (type == MEM_U) {
        start=KERNEL_RESERVED/CHUNK_SIZE;
        end=MAX_CHUNK;
    } else {
        printk("Unknown allocation type %d\n", type);
        return NULL;
    }

    acquire(&MEM_LOCK);

    first_chunk=find_free(n_chunks,start,end);

    if (first_chunk < 0) {
        printk("Failed to allocate %d chunks\n", n_chunks);
        release(&MEM_LOCK);
        return NULL;
    }

    for (i = 0; i < n_chunks; i++) {
        mark_used(first_chunk + i);
    }

    release(&MEM_LOCK);

    memset((void *)(first_chunk*CHUNK_SIZE),0,n_chunks*CHUNK_SIZE);

    return (void *)(first_chunk*CHUNK_SIZE);
}


int32_t relinquish(unsigned long start, uint32_t n_chunks)
{
    int i;
    int first_chunk;

    first_chunk=(int)start/CHUNK_SIZE;

    memset(start, 'V', n_chunks*CHUNK_SIZE);

    for (i = 0; i < n_chunks; i++) {
        mark_free(first_chunk + i);
    }

    return 0;
}

void memory_init(unsigned long mem_kernel)
{

}
