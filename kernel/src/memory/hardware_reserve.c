#include <stdlib.h>
#include <memory/hardware_reserve.h>

#define MAX_MEMORY (1024*1024*1024)     // 1GB
#define KERNEL_RESERVED (16*1024*1024)  // 16MB

#define CHUNK_SIZE 4096

lock_t MEM_LOCK = {UNLOCKED, KERNEL_OWNER};

uint32_t MEMORY_TOTAL;

unsigned long MEMORY_MAP[MAX_MEMORY/CHUNK_SIZE/32];

uint32_t MAX_CHUNK = 0;

uint64_t MEM_OFFSET;

#define mark_used(chunk) \
    set_bit((MEMORY_MAP), chunk)

#define mark_free(chunk) \
    clear_bit((MEMORY_MAP), (chunk))

#define test_used(chunk) \
    get_bit((MEMORY_MAP), (chunk))

static int mem_init(unsigned long memory_total,
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
        return 0;
    }

    //acquire(&MEM_LOCK);

    first_chunk=find_free(n_chunks,start,end);

    if (first_chunk < 0) {
        printk("Failed to allocate %d chunks\n", n_chunks);
        //release(&MEM_LOCK);
        return 0;
    }

    for (i = 0; i < n_chunks; i++) {
        mark_used((first_chunk + i));
    }

    //release(&MEM_LOCK);

    memset((unsigned char*)(first_chunk*CHUNK_SIZE+MEM_OFFSET),0,n_chunks*CHUNK_SIZE);

    return (unsigned long)(first_chunk*CHUNK_SIZE+MEM_OFFSET);
}


int32_t relinquish(unsigned long start, uint32_t n_chunks)
{
    int i;
    int first_chunk;

    first_chunk=(int)(start-MEM_OFFSET)/CHUNK_SIZE;

    memset((uint64_t *)(start), 'V', n_chunks*CHUNK_SIZE);

    for (i = 0; i < n_chunks; i++) {
        mark_free((first_chunk + i));
    }

    return 0;
}

void memory_init(unsigned long mem_kernel)
{
    uint64_t start,length,i;

    MEMORY_TOTAL = MAX_MEMORY;

    mem_init(MEMORY_TOTAL, mem_kernel);
}
