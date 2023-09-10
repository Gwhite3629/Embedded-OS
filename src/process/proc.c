#include "../../include/memory/malloc.h"
#include "../../include/process/proc.h"
#include "../../include/process/status.h"
#include "../../include/stdlib/err.h"
#include "../../include/stdlib/syscall.h"

proc_t *first_proc = NULL;

proc_t *current_proc;

static proc_t all_procs[MAX_PROCESSES];

err_t proc_insert(proc_t *p)
{
    err_t ret = E_NOERR;

    current_proc = p;

    // Clear TLB
    asm ("mov r4, #0\n"
         "mcr p15, 0, r4, c8, c7, 0");

    load_context(&(*p).registers);

    (*p).status = RUNNING;

    return ret;
}

err_t proc_remove(void)
{
    err_t ret = E_NOERR;

    store_context(&(*current_proc).registers);

    (*current_proc).status = WAITING;

    return ret;
}

err_t proc_queue(proc_t *p);    // Puts proc in queue, sets idle -> waiting

proc_t *proc_construct(void)
{
    error = E_NOERR;
    int i;
    int pid = -1;

    proc_t *p = NULL;

    for (i = 0; i < MAX_PROCESSES; i++) {
        if (all_procs[i].status == INVALID) {
            p = &all_procs[i];
            pid = i;
            break;
        }
    }

    if (pid == -1) {
        error = E_NOSTART;
        return NULL;
    }

    p = pmalloc();
    PROC_CHECK(p);

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

    return p;
}

err_t proc_destroy(proc_t *p);           // Puts proc in destroy queue, sets destroy

err_t proc_sleep(proc_t *p);             // Puts proc in sleep, sets (any) -> sleep

err_t proc_sweep();                      // Sweeps to look for stale processes and clean

void proc_exit(proc_t *p, err_t err);              // Exit proc on error