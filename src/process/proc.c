#include "../../include/memory/malloc.h"
#include "../../include/process/proc.h"
#include "../../include/process/status.h"
#include "../../include/stdlib/err.h"
#include "../../include/stdlib/syscall.h"

proc_t *first_proc = NULL;

proc_t *current_proc;

static proc_t all_procs[MAX_PROCESSES];

err_t proc_insert(proc_t **p)
{
    err_t ret = E_NOERR;

    asm (
            "mov    r2, %[reg]\n"       // Loads address of first register
            "ldr    r0, [r2, #60]\n"    // 
            "msr    SPSR, r0\n"         // Restore SPSR
            "ldmia  r2, {r0-r14}\n"     // Restore registers
            "mov    pc, lr\n"           // Return and restore pc
            :
            :   [reg] "r"(&((*p)->registers.r[0]))
            :
    );

    (*p)->status = RUNNING;

    return ret;
}

err_t proc_remove(proc_t **p)
{
    err_t ret = E_NOERR;

    asm (
            "mov    r2, %[reg]\n"       // Load register 0
            "stmia  r2, {r0-lr}\n"      // Save registers to LR
            "add    r2, r2, #60\n"      // Increment to PC
            "mrs    r0, SPSR\n"         // Load SPSR
            "stmia  r2, {r0}\n"         // Store
            :
            :   [reg] "r"(&((*p)->registers.r[0]))
            :
    );

    (*p)->status = WAITING;

    return ret;
}

err_t proc_queue(proc_t *p);    // Puts proc in queue, sets idle -> waiting

err_t proc_construct(void)
{
    err_t ret = E_NOERR;
    int i;

    proc_t *p = NULL;

    for (i = 0; i < MAX_PROCESSES; i++) {
        if (all_procs[i].status == 0) {
            p = &all_procs[i];
            break;
        }
    }

    p = pmalloc();
    PROC_CHECK(p);

    if (p == NULL) {
        proc_exit(p, E_NOMEM);
    }

    memset(p, 0, sizeof(proc_t));

    p->status = IDLE;
    p->pid = i;

    p->stack = NULL;
    p->text = NULL;
    p->stacksize = 0;
    p->textsize = 0;

    for (i = 0; i < 14; i++) {
        p->registers.r[i] = 0;
    }

    p->registers.spsr = 0x10;
    p->registers.r[14] = 0;

    return ret;
}

err_t proc_destroy(proc_t *p);           // Puts proc in destroy queue, sets destroy

err_t proc_sleep(proc_t *p);             // Puts proc in sleep, sets (any) -> sleep

err_t proc_sweep();                      // Sweeps to look for stale processes and clean

void proc_exit(proc_t *p, err_t err);              // Exit proc on error