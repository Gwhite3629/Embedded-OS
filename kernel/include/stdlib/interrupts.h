#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "types.h"

err_t handle_irq_event(irq_t * q);

// Int indicates number for verification
// Void * indicates pointer to data structure for relevant irq
typedef err_t (*irq_handler_t)(int, void *);

typedef struct irq_t {
    irq_handler_t func;
    unsigned int irq;
    lock_t irq_lock;
    void *handler_data;
} irq_t;

static inline uint32_t get_CPSR(void)
{
    uint32_t temp;

    asm volatile ("mrs %0, CPSR":"=r" (temp):);

    return temp;
}

static inline void set_CPSR(uint32_t new_cpsr)
{
    asm volatile ("mrs CPSR_cxsf,%0"::"r"(new_cpsr));
}

static inline void enable_interrupts(void)
{
    uint32_t temp;
    temp = get_CPSR();
    set_CPSR(temp & ~0x80);
}

static inline uint32_t disable_interrupts(void)
{
    uint32_t temp;
    temp = get_CPSR();
    set_CPSR(temp | 0x80);
    return temp;
}

#endif // _INTERRUPTS_H_