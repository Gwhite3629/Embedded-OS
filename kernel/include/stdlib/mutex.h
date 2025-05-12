#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "lock.h"

typedef struct mutex {
    int64_t owner;
    spinlock_t wait_lock;
    struct list_node_t wait_list;
} mutex_t;

#endif // _MUTEX_H_
