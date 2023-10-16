#ifndef _LOCK_H_
#define _LOCK_H_

#include "types.h"

#define KERNEL_OWNER 0

#define LOCKED 1
#define UNLOCKED 0

typedef struct lock_t {
    bool acquired;
    uint32_t owner;
} lock_t;

err_t init_lock(lock_t *lock);

void remove_lock(lock_t *lock);

void acquire(lock_t *lock);

void release(lock_t *lock);

void wait_acquire(lock_t *lock);

#endif // _LOCK_H_