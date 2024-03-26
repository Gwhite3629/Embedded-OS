#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "message.h"
#include "bitmap.h"

extern TRIGGER_array TQ;
extern uint32_t TQ_size;
extern MESSAGE_array DQ;
extern uint32_t DQ_size;
extern MESSAGE_array LPQ;
extern uint32_t LPQ_size;

// Placeholder for driver functions
extern void rawGetHeader(uint8_t *b);
extern void rawGetBytes(uint8_t **b, size_t n);

int getMessage(void);

int triggerInsert(uint8_t node, uint8_t trigger, uint8_t *message, uint32_t messageSize);

int demandInsert(uint8_t node, uint8_t *message, bool tb, uint32_t messageSize);

// Two checks to promote lowprio,
// 1. End of this function, check if DQ is empty, if so then promote all
// 2. End of DQ eject function, check if DQ is empty, if so then promote all
int lowprioInsert(uint8_t node, uint8_t *message, uint32_t messageSize);

int triggerExecute(uint8_t node, uint8_t trigger);

#endif // _QUEUE_H_