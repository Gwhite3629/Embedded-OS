#include <stdlib.h>
#include "spinlock.h"
#include "semaphore.h"

void semaphore_init(semaphore_t *sem, char *name, uint32_t n);
{
    sem->lock = SPINLOCK_INIT;
    sem->count = n;
    sem->wait_list = list_create();
    list_push(sem->wait_list, name);
}


static inline int ___down(semaphore_t *sem, int64_t state, int64_t timeout)
{
    
}

void semaphore_down(semaphore_t *sem)
{
    spinlock_acquire(&sem->lock);
    if (likely(sem->count > 0)) {
        sem->count--;
    } else {
        
    }
    spinlock_release(&sem->lock);
}

int mustcheck noinline semaphore_down_preempt(semaphore_t *sem)
{

}

int mustcheck noinline semaphore_down_kill(semaphore_t *sem)
{

}

int mustcheck noinline semaphore_down_trylock(semaphore_t *sem)
{

}

int mustcheck noinline semaphore_down_timeout(semaphore_t *sem, uint64_t ticks)
{

}

void semaphore_up(semaphore_t *sem)
{

}
