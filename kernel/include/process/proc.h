#ifndef _PROC_H_
#define _PROC_H_

#include "../stdlib/err.h"
#include "../stdlib/types.h"
#include "../memory/mmu_func.h"

#define MAX_PROCESSES 20
#define DEFAULT_STACK 8192

// Process structure
typedef struct proc_t {
    uint32_t r[17]; // Programmable registers        
    // 68 bytes

    uint32_t status;    // 72 bytes
    uint32_t pid;       // 76 bytes

    void *stack;        // 80 bytes
    uint32_t stacksize; // 84 bytes

    void *heap;

    void *text;         // 96 bytes
    uint32_t textsize;  // 100 bytes

    void *data;         // 104 bytes
    uint32_t datasize;  // 108 bytes

    void *bss;          // 112 bytes
    uint32_t bsssize;   // 116 bytes

    MM_t mm;            // 132 bytes

    //uint32_t pad[3];
    
} proc_t;   // 132 bytes in size

extern proc_t *current_proc;

extern proc_t *first_proc;

err_t proc_insert(proc_t *p);   // Puts proc on processor, sets running

err_t proc_remove(void);   // Puts proc in queue from processor, sets waiting

err_t proc_queue(proc_t *p);    // Puts proc in queue, sets idle -> waiting

proc_t *proc_construct(void);         // Puts proc in queue, sets idle

err_t proc_destroy(proc_t *p);           // Puts proc in destroy queue, sets destroy

err_t proc_sleep(proc_t *p);             // Puts proc in sleep, sets (any) -> sleep

err_t proc_sweep();                      // Sweeps to look for stale processes and clean

void proc_exit(proc_t *p, err_t err);              // Exit proc on error

#define PROC_CHECK(err) \
    if (err == NULL) { \
        proc_exit(p, E_NOMEM); \
        return NULL; \
    }

void load_context(unsigned int *regs);
void store_context(unsigned int *regs);

#endif  // _PROC_H_
