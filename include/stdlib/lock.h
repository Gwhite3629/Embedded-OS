#ifndef _LOCK_H_
#define _LOCK_H_

#include "types.h"

typedef struct lock_t {
    bool acquired;
    uint32_t owner;
} lock_t;

void acquire(lock_t *lock);

void release(lock_t *lock);

void wait_acquire(lock_t *lock);

#endif // _LOCK_H_