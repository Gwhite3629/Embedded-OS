#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "err.h"
#include "types.h"

// Int indicates number for verification
// Void * indicates pointer to data structure for relevant irq
typedef err_t (*irq_handler_t)(int, void*);

typedef struct irq_t {
    irq_handler_t func;
    unsigned int irq;
    void* handler_data;
} irq_t;

extern irq_t IRQ_LIST[1];

void init_tickcounter();
int get_tickcounter();

void handle_error(uint64_t x0);

void handle_irq(void);

void init_vectors(void);

void enable_interrupts(void);

void disable_interrupts(void);

void memory_barrier(void);

void interrupt_barrier(void);

void instruction_barrier(void);

#endif // _INTERRUPTS_H_
