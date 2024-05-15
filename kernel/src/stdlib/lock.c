#include <stdlib.h>
#include <process/proc.h>

err_t init_lock(lock_t *lock)
{
    lock->owner = -1;
    lock->acquired = 0;
}

void remove_lock(lock_t *lock)
{
    if (READ_ONCE(lock->owner) == current_proc->pid) {
        lock->owner = -1;
        lock->acquired = 0;
    }
}

void acquire(lock_t *lock)
{
    unsigned long tmp;
    uint32_t newval;
    lock_t lockval;

    if (READ_ONCE(lock->owner) == current_proc->pid) {
        if (!READ_ONCE(lock->acquired)) {
            WRITE_ONCE(lock->acquired, 1);
        } else {
            return;
        }
    } else {
        wait_acquire(lock);
    }
    
}

void release(lock_t *lock)
{
    if (READ_ONCE(lock->owner) == current_proc->pid) {
        WRITE_ONCE(lock->acquired, 0);
    } else {
        wait_acquire(lock);
        release(lock);
    }
}

void wait_acquire(lock_t *lock)
{
    if (READ_ONCE(lock->owner) == current_proc->pid) {
        WRITE_ONCE(lock->acquired, 1);
    } else {
        while (!(READ_ONCE(lock->owner) == current_proc->pid));
        WRITE_ONCE(lock->owner, current_proc->pid);
        WRITE_ONCE(lock->acquired, 1);
    }
}