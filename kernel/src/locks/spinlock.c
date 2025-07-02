#include <locks/spinlock.h>
#include <stdlib.h>

void noinline spinlock_acquire(spinlock_t *lock)
{
    uint64_t tmp = 0;
    uint32_t newval = 0;
    spinlock_t lockval;

    asm volatile (
    "1: ldrex   %0, [%3]\n"
    "   add     %1, %0, %4\n"
    "   strex   %2, %1, [%3]\n"
    "   teq     %2, #0\n"
    "   bne 1b"
    : "=&r" (lockval), "=&r" (newval), "=&r" (tmp)
    : "r" (&lock->slock), "I" (1 << TICKET_SHIFT)
    : "cc"
    );

    while (lockval.tickets.next != lockval.tickets.owner) {
        wfe();
        lockval.tickets.owner = READ_ONCE(lock->tickets.owner);
    }

    memory_barrier();
}

int noinline spin_trylock(spinlock_t *lock)
{
    uint64_t contended;
    uint64_t res;
    uint32_t slock;

    do {
        asm volatile (
        "	ldrex	%0, [%3]\n"
		"	mov	%2, #0\n"
		"	subs	%1, %0, %0, ror #16\n"
		"	addeq	%0, %0, %4\n"
		"	strexeq	%2, %0, [%3]"
		: "=&r" (slock), "=&r" (contended), "=&r" (res)
		: "r" (&lock->slock), "I" (1 << TICKET_SHIFT)
		: "cc"
        );
    } while (res);

    if (!contended) {
        memory_barrier();
        return 1;
    } else {
        return 0;
    }
}

void noinline spin_unlock(spinlock_t *lock)
{
    memory_barrier();
    lock->tickers.owner++;
    interrupt_barrier();
}

int noinline spinlock_is_unlocked(spinlock_t lock)
{
    return lock.tickets.owner == lock.tickets.next;
}

int noinline spinlock_is_locked(spinlock_t *lock)
{
    return !spinlock_is_unlocked(READ_ONCE(*lock));
}

int noinline spinlock_is_contended(spinlock_t *lock)
{
    struct __raw_tickets tickets = READ_ONCE(lock->tickets);
    return (tickets.next - tickets.owner) > 1;
}
