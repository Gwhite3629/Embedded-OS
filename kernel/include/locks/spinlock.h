#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#define wfe(cond) asm volatile (    \
    "it " cond "\n\t"               \
    "wfe" cond ".n"                 \
                                    \
    "nop.w"                         \
)

typedef struct {
    uint32_t slock;
    struct _raw_tickets {
        uint16_t owner;
        uint16_t next;
    } tickets;
} spinlock_t;

#define TICKET_SHIFT 16

#define SPINLOCK_INIT { { 0 } }

void spinlock_acquire(spinlock_t *lock);
int spin_trylock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
int spinlock_is_unlocked(spinlock_t lock);
int spinlock_is_locked(spinlock_t *lock);
int spinlock_is_contended(spinlock_t *lock);

#endif // _SPINLOCK_H_