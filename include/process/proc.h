#ifndef _PROC_H_
#define _PROC_H_

#include "../stdlib/err.h"

// Process structure
typedef struct proc {
    struct {
        uint32_t r[16]; // Programmable registers
        uint32_t cpsr;  // Current Program Status Register
        uint32_t spsr;
    } registers;        // 72 bytes

    uint32_t status;    // 76 bytes
    uint32_t pid;       // 80 bytes

    void *stack;        // 84 bytes
    uint32_t stacksize; // 98 bytes

    void *heap;         // 92 bytes
    uint32_t heapsize;  // 96 bytes

    void *text;         // 100 bytes
    uint32_t textsize;  // 104 bytes

    void *data;
    uint32_t datasize;  // 108 bytes

    void *bss;
    uint32_t bsssize;   // 112 bytes

    uint32_t pad[4];
    
} proc_t;   // 128 bytes in size

err_t proc_insert(proc_t *p);   // Puts proc on processor, sets running

err_t proc_remove(proc_t *p);   // Puts proc in queue from processor, sets waiting

err_t proc_queue(proc_t *p);    // Puts proc in queue, sets idle -> waiting

err_t proc_construct(proc_t *p);         // Puts proc in queue, sets idle

err_t proc_destroy(proc_t *p);           // Puts proc in destroy queue, sets destroy

err_t proc_sleep(proc_t *p);             // Puts proc in sleep, sets (any) -> sleep

err_t proc_sweep();                      // Sweeps to look for stale processes and clean

#endif  // _PROC_H_