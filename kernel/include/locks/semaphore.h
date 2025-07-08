#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "spinlock.h"

typedef struct {
    spinlock_t lock;
    uint32_t count;
    list_t wait_list;
} semaphore_t;

void semaphore_init(semaphore_t* sem, char* name, uint32_t n);
void semaphore_down(semaphore_t* sem);
int semaphore_down_preempt(semaphore_t* sem);
int semaphore_down_kill(semaphore_t* sem);
int semaphore_down_trylock(semaphore_t* sem);
int semaphore_down_timeout(semaphore_t* sem, int64_t ticks);
void semaphore_up(semaphore_t* sem);

#endif // _SEMAPHORE_H_
