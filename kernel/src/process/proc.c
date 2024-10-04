#include <memory/malloc.h>
#include <process/proc.h>
#include <process/status.h>
#include <stdlib.h>

proc_t *first_proc = NULL;

proc_t *current_proc;

static proc_t all_procs[MAX_PROCESSES];


void store_context(unsigned int *regs);
/*
{
    asm volatile (
        "mov    r2, %[save]\n"
        "stmia  r2, {r0-lr}\n"
        "add    r2, r2, #60\n"
        "mrs    r0, SPSR\n"
        "stmia  r2, {r0}\n"
        :
        : [save] "r"(regs)
        :
    );
}
*/

void load_context(unsigned int *regs);
/*
{
    asm volatile (
        "mov r2, %[restore]\n"
        "ldr r0, [r2, #60]\n"
        "msr SPSR, r0\n"
        "ldmia r2, {r0-r14}\n"
        "mov pc, lr\n"
        :
        : [restore] "r"(regs)
        :
    );
}
*/

err_t proc_insert(proc_t *p);
/*
{
    err_t ret = E_NOERR;

    current_proc = p;

    // Clear TLB
    asm ("mov r4, #0\n"
         "mcr p15, 0, r4, c8, c7, 0");

    load_context(p->r);

    (*p).status = RUNNING;

    return ret;
}
*/

err_t proc_remove(void);
/*
{
    err_t ret = E_NOERR;

    store_context(current_proc->r);

    (*current_proc).status = WAITING;

    return ret;
}
*/

err_t proc_queue(proc_t *p);    // Puts proc in queue, sets idle -> waiting

proc_t *proc_construct(void)
{
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
        ret = E_NOSTART;
        return NULL;
    }

    //new(p, 1, proc_t);
    PROC_CHECK(p);

    //memset(p, 0, sizeof(proc_t));

    p->status = IDLE;
    p->pid = i;

    p->stack = NULL;
    p->text = NULL;
    p->stacksize = 0;
    p->textsize = 0;

    for (i = 0; i < 14; i++) {
        p->r[i] = 0;
    }

    p->r[16] = 0x10;
    p->r[14] = 0;

    return p;
}

err_t proc_destroy(proc_t *p);           // Puts proc in destroy queue, sets destroy

err_t proc_sleep(proc_t *p);             // Puts proc in sleep, sets (any) -> sleep

err_t proc_sweep();                      // Sweeps to look for stale processes and clean

// Exit proc on error
void proc_exit(proc_t *p, err_t err)
{
    return;
}