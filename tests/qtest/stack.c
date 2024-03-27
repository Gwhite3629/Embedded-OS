#include "stack.h"

int init_stack(stack_t *stack, uint8_t node, uint8_t *message, uint32_t messageSize)
{
    int ret = SUCCESS;
    
    if (stack->data == NULL) {
        new(stack->data, 1, MESSAGE_container);
    }
    stack->size = 1;
    new(stack->data[0].message, messageSize, uint8_t);
    memcpy(stack->data[0].message, message, messageSize);
    stack->data[0].messageSize = messageSize;
    stack->data[0].node = node;

exit:
    return ret;
}

int push_back(stack_t *stack, uint8_t node, uint8_t *message, uint32_t messageSize)
{
    int ret = SUCCESS;

    stack->size++;
    alt(stack->data, stack->size, MESSAGE_container);
    new(stack->data[stack->size-1].message, messageSize, uint8_t);
    memcpy(stack->data[stack->size-1].message, message, messageSize);
    stack->data[stack->size-1].messageSize = messageSize;
    stack->data[stack->size-1].node = node;

exit:
    return ret;
}

int push(stack_t *stack, uint8_t node, uint8_t *message, uint32_t messageSize)
{

    int ret = SUCCESS;

    stack->size++;
    alt(stack->data, stack->size, MESSAGE_container);

    for (unsigned int i = stack->size; i > 1; i--) {
        stack->data[i-1].message = stack->data[i-2].message;
        stack->data[i-1].messageSize = stack->data[i-2].messageSize;
        stack->data[i-1].node = stack->data[i-2].node;
    }
    new(stack->data[0].message, messageSize, uint8_t);
    memcpy(stack->data[0].message, message, messageSize);
    stack->data[0].messageSize = messageSize;
    stack->data[0].node = node;

exit:
    return ret;
}