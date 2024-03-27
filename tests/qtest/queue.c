#include "message.h"
#include "bitmap.h"
#include "queue.h"
#include "memory.h"

#include "stack.h"

TRIGGER_array TQ;
uint32_t TQ_size;
stack_t DQ_stack = {NULL, 0};
stack_t LPQ_stack = {NULL, 0};

int getMessage(void)
{
    int ret = SUCCESS;

    uint8_t byteOne;
    uint8_t nodeNumber;
    uint8_t triggerNumber;
    uint8_t nExtraHeaders;
    uint8_t *extraHeaders = NULL;
    uint8_t *message = NULL;
    uint8_t messageSize;

    rawGetHeader(&byteOne);
    rawGetHeader(&nodeNumber);

    uint8_t mType = (byteOne & 0xC0) >> 6; // 0b11000000

    if (mType == 0) {
        rawGetHeader(&triggerNumber);
    }
    if (mType == 3) {
        triggerNumber = byteOne & 0x3F; // 0b00111111
    } else {
        nExtraHeaders = (byteOne & 0x30) >> 4; // 0b00110000
        new(extraHeaders, nExtraHeaders, uint8_t);
        rawGetBytes(&extraHeaders, nExtraHeaders);
        messageSize = byteOne & 0x0F; // 0b00001111
        for (int i = 0; i < nExtraHeaders; i++) {
            messageSize += extraHeaders[i];
        }
        new(message, messageSize, uint8_t);
        rawGetBytes(&message, messageSize);
    }

    switch (mType) {
        case 0: // Trigger message
            triggerInsert(nodeNumber, triggerNumber, message, messageSize);
            break;
        case 1: // Demand message
            demandInsert(nodeNumber, message, 0, messageSize);
            break;
        case 2: // Low Priority
            lowprioInsert(nodeNumber, message, messageSize);
            break;
        case 3: // Trigger
            triggerExecute(nodeNumber, triggerNumber);
            break;
        default:
            printf("Failed\n");
    }

exit:
    return ret;
}

int triggerInsert(uint8_t node, uint8_t trigger, uint8_t *message, uint32_t messageSize)
{
    int ret = SUCCESS;
    printf("Found: Trigger Message\n");
    TRIGGER_container *container;
    TRIGGER_container *temp;

    if ((TQ_size == 0) && (TQ == NULL)) {
        new(TQ, 1, TRIGGER_container);
        TQ[0].bitmap[0] = 0; // Set lower 32 bits to 0
        TQ[0].bitmap[1] = 0; // Set upper 32 bits to 0
        memset(TQ[0].messages, 0, 64 * sizeof(message_t));
        set_bit(TQ[0].bitmap, trigger);
        TQ[0].node = node;
        new(TQ[0].messages[trigger], messageSize, uint8_t);
        memcpy(TQ[0].messages[trigger], message, messageSize);
        TQ[0].messageSize[trigger] = messageSize;
        TQ_size++;
        goto exit;
    } else if ((TQ_size == 0) && (TQ != NULL)) {
        TQ[0].bitmap[0] = 0; // Set lower 32 bits to 0
        TQ[0].bitmap[1] = 0; // Set upper 32 bits to 0
        memset(TQ[0].messages, 0, 64 * sizeof(message_t));
        set_bit(TQ[0].bitmap, trigger);
        TQ[0].node = node;
        new(TQ[0].messages[trigger], messageSize, uint8_t);
        memcpy(TQ[0].messages[trigger], message, messageSize);
        TQ[0].messageSize[trigger] = messageSize;
        TQ_size++;
        goto exit;
    }

    temp = lookup(node, TQ, TQ_size);
    if (temp != NULL) { // Node is present
        printf("Node Present\n");
        if (get_bit((*temp).bitmap, trigger)) {
            CHECK(triggerExecute(node, trigger));
        } else {
            set_bit((*temp).bitmap, trigger);
            new((*temp).messages[trigger], messageSize, uint8_t);
            memcpy((*temp).messages[trigger], message, messageSize);
            (*temp).messageSize[trigger] = messageSize;
            goto exit;
        }
    }

    CHECK(insert(&TQ, node, &TQ_size));
    container = lookup(node, TQ, TQ_size);
    (*container).bitmap[0] = 0;
    (*container).bitmap[1] = 0;
    memset((*container).messages, 0, 64*sizeof(message_t));
    set_bit((*container).bitmap, trigger);
    (*container).node = node;
    new((*container).messages[trigger], messageSize, uint8_t);
    memcpy((*container).messages[trigger], message, messageSize);
    (*container).messageSize[trigger] = messageSize;

exit:
    return ret;
}

int demandInsert(uint8_t node, uint8_t *message, bool tb, uint32_t messageSize)
{
    printf("Found: Demand\n");
    int ret = SUCCESS;

    if ((DQ_stack.size == 0)) {
        CHECK(init_stack(&DQ_stack, node, message, messageSize));
        goto exit;
    }

    if (tb) {
        CHECK(push(&DQ_stack, node, message, messageSize));
    } else {
        CHECK(push_back(&DQ_stack, node, message, messageSize));
    }

exit:
    return ret;
}

// Two checks to promote lowprio,
// 1. End of this function, check if DQ is empty, if so then promote all
// 2. End of DQ eject function, check if DQ is empty, if so then promote all
int lowprioInsert(uint8_t node, uint8_t *message, uint32_t messageSize)
{
    printf("Found: Low Priority\n");
    int ret = SUCCESS;

    if ((LPQ_stack.size == 0)) {
        CHECK(init_stack(&LPQ_stack, node, message, messageSize));
    } else {
        CHECK(push_back(&LPQ_stack, node, message, messageSize));
    }

    // Promote all if DQ is empty
    if (DQ_stack.size == 0) {
        for (uint32_t i = 0; i < LPQ_stack.size; i++) {
            demandInsert(LPQ_stack.data[i].node, LPQ_stack.data[i].message, 0, LPQ_stack.data[i].messageSize);
        }
        alt(LPQ_stack.data, 1, MESSAGE_container);
        LPQ_stack.size = 0;
    }

exit:
    return ret;
}

int triggerExecute(uint8_t node, uint8_t trigger)
{
    printf("Found: Trigger Execute\n");
    int ret = SUCCESS;

    TRIGGER_container *temp;

    temp = lookup(node, TQ, TQ_size);
    if (temp == NULL) {
        ret = -1;
        goto exit;
    }

    if (get_bit((*temp).bitmap, trigger)) {
        demandInsert((*temp).node, (*temp).messages[trigger], 1, (*temp).messageSize[trigger]);
        clear_bit((*temp).bitmap, trigger);
        if (((*temp).bitmap[0] == 0) && ((*temp).bitmap[1] == 0)) {
            remove_node(&TQ, node, &TQ_size);
        }
    } else {
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}
