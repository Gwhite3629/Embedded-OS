#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "types.h"

// Int indicates number for verification
// Void * indicates pointer to data structure for relevant irq
typedef err_t (*irq_handler_t)(int, void *);

typedef struct irq_t {
    irq_handler_t func;
    unsigned int irq;
    lock_t irq_lock;
    void *handler_data;
} irq_t;

err_t handle_irq_event(irq_t * q);

void init_vectors(void);

void enable_interrupts(void);

void disable_interrupts(void);

void interrupt_barrier(void);

#endif // _INTERRUPTS_H_
