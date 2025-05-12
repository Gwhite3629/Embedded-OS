#ifndef _LOCK_H_
#define _LOCK_H_

#include "types.h"

typedef struct spinlock {
    volatile uint32_t lock;
} spinlock_t;

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif // _LOCK_H_
