#include <stdlib.h>

/* * * * * * *
 * SPINLOCKS *
 * * * * * * */

void __attribute__((noinline)) spinlock_acquire(spinlock_t *lock)
{
    unsigned long tmp;
    uint32_t newval;
    spinlock_t lockval;

    asm volatile (
    "1: ldrex   %0, [%3]\n"
    "   add     %1, %0, %4\n"
    "   strex   %2, %1, [%3]\n"
    "   teq     %2, #0\n"
    "   bne     1b"
    :   "=&r"   (lockval), "=&r" (newval), "=&r" (tmp)
    :   "r"     (&lock->), "I" (1 << TICKET_SHIFT)
    :   "cc");
}

int __attribute__((noinline)) spinlock_trylock(spinlock_t *lock)
{

}

void __attribute__((noinline)) spinlock_release(spinlock_t *lock)
{

}

/* * * * * *
 * RWLOCKS *
 * * * * * */

// Write

void __attribute__((noinline)) writelock_acquire(rwlock_t *rw)
{

}

int  __attribute__((noinline)) writelock_trylock(rwlock_t *rw)
{

}

void __attribute__((noinline)) writelock_release(rwlock_t *rw)
{

}

// Read

void __attribute__((noinline)) readlock_acquire(rwlock_t *rw)
{

}

int  __attribute__((noinline)) readlock_trylock(rwlock_t *rw)
{

}

void __attribute__((noinline)) readlock_release(rwlock_t *rw)
{

}

