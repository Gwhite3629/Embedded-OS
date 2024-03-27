#ifndef _STACK_H_
#define _STACK_H_

#include <stdint.h>

#include "memory.h"
#include "message.h"
#include "utils.h"

typedef struct stack_t {
    MESSAGE_container *data;
    uint32_t size;
} stack_t;

int init_stack(stack_t *stack, uint8_t node, uint8_t *message, uint32_t messageSize);

int push(stack_t *stack, uint8_t node, uint8_t *message, uint32_t messageSize);

int push_back(stack_t *stack, uint8_t node, uint8_t *message, uint32_t messageSize);

#endif // _STACK_H_