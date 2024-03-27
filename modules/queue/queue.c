#include "message.h"
#include "bitmap.h"
#include "queue.h"

int getMessage(void)
{
    int ret = SUCCESS;

    uint8_t byteOne;
    uint8_t nodeNumber;
    uint8_t triggerNumber;
    uint8_t nExtraHeaders;
    uint8_t *extraHeaders;
    uint8_t *message;
    uint8_t messageSize;

    rawGetHeader(&byteOne);
    rawGetHeader(&nodeNumber);

    uint8_t mType = byteOne && 0b11000000;

    if (mType == 0) {
        rawGetHeader(&triggerNumber);
    }
    if (mType == 3) {
        triggerNumber = byteOne & 0b00111111;
    } else {
        nExtraHeaders = 0b00110000;
        MEM(extraHeaders, nExtraHeaders, uint8_t);
        rawGetBytes(extraHeaders, nExtraHeaders);
        messageSize = byteOne & 0b00001111;
        for (int i = 0; i < nExtraHeaders; i++) {
            messageSize += extraHeaders[i];
        }
        MEM(message, messageSize, uint8_t);
        rawGetBytes(message, messageSize);
        triggerInsert(nodeNumber, triggerNumber, message);
    }

    switch (byteOne && 0b11000000) {
        case 0: // Trigger message
            triggerInsert(nodeNumber, triggerNumber, message);
            break;
        case 1: // Demand message
            demandInsert(nodeNumber, message, 0);
            break;
        case 2: // Low Priority
            lowprioInsert(nodeNumber, message);
            break;
        case 3: // Trigger
            triggerExecute(nodeNumber, triggerNumber);
            break;
    }

exit:
    return ret;
}

int triggerInsert(uint8_t node, uint8_t trigger, uint8_t *message)
{
    int ret = SUCCESS;
    TRIGGER_container *container;
    TRIGGER_container *temp;

    if ((TQ_size == 0) && (TQ == NULL)) {
        MEM(TQ, 1, TRIGGER_container);
        TQ[0].bitmap[0] = 0; // Set lower 32 bits to 0
        TQ[0].bitmap[1] = 0; // Set upper 32 bits to 0
        memset(TQ[0].messages, 0, 64 * sizeof(message_t));
        set_bit(TQ[0].bitmap, trigger);
        TQ[0].node = node;
        TQ[0].messages[trigger] = message;
        TQ_size++;
        goto exit;
    } else if ((TQ_size == 0) && (TQ != NULL)) {
        TQ[0].bitmap[0] = 0; // Set lower 32 bits to 0
        TQ[0].bitmap[1] = 0; // Set upper 32 bits to 0
        memset(TQ[0].messages, 0, 64 * sizeof(message_t));
        set_bit(TQ[0].bitmap, trigger);
        TQ[0].node = node;
        TQ[0].messages[trigger] = message;
        TQ_size++;
        goto exit;
    }

    temp = lookup(node, TQ, TQ_size);
    if (temp != NULL) { // Node is present
        if (get_bit((*temp).bitmap, trigger)) {
            CHECK(triggerExecute(node, trigger));
        } else {
            set_bit((*temp).bitmap, trigger);
            (*temp).messages[trigger] = message;
        }
    }

    CHECK(insert(&TQ, node, &TQ_size));
    container = lookup(node, TQ, TQ_size);
    (*container).bitmap[0] = 0;
    (*container).bitmap[1] = 0;
    memset((*container).messages, 0, 64);
    set_bit((*container).bitmap, trigger);
    (*container).node = node;
    (*container).messages[trigger] = message;
    TQ_size++;

exit:
    return ret;
}

int demandInsert(uint8_t node, uint8_t *message, bool tb)
{
    int ret = SUCCESS;

    MESSAGE_container *temp;

    if ((DQ_size == 0) && (DQ == NULL)) {
        DQ_size = 1;
        MEM(DQ, 1, MESSAGE_container);
        DQ[0].message = message;
        DQ[0].node = node;
        goto exit;
    } else if ((DQ_size == 0) && (DQ != NULL)) {
        DQ_size = 1;
        DQ[0].message = message;
        DQ[0].node = node;
        goto exit;
    } else {
        DQ_size++;
        MEM_(DQ, DQ_size, MESSAGE_container);
    }

    if (tb) {
        MEM(temp, DQ_size-1, MESSAGE_container);
        memcpy(temp, DQ, (DQ_size-1) * sizeof(MESSAGE_container));
        memcpy(DQ + sizeof(MESSAGE_container), temp, (DQ_size-1) * sizeof(MESSAGE_container));
        DQ[0].message = message;
        DQ[0].node = node;
    } else {
        DQ[DQ_size-1].message = message;
        DQ[DQ_size-1].node = node;
    }

exit:
    return ret;
}

// Two checks to promote lowprio,
// 1. End of this function, check if DQ is empty, if so then promote all
// 2. End of DQ eject function, check if DQ is empty, if so then promote all
int lowprioInsert(uint8_t node, uint8_t *message)
{
    int ret = SUCCESS;

    if ((LPQ_size == 0) && (LPQ == NULL)) {
        LPQ_size = 1;
        MEM(LPQ, 1, MESSAGE_container);
        LPQ[0].message = message;
        LPQ[0].node = node;
        goto exit;
    } else if ((LPQ_size == 0) && (LPQ != NULL)) {
        LPQ_size = 1;
        LPQ[0].message = message;
        LPQ[0].node = node;
        goto exit;
    } else {
        LPQ_size++;
        MEM_(LPQ, LPQ_size, MESSAGE_container);
        LPQ[DQ_size-1].message = message;
        LPQ[DQ_size-1].node = node;
    }

    // Promote all if DQ is empty
    if (DQ_size == 0) {
        for (int i = 0; i < LPQ_size; i++) {
            demandInsert(LPQ[i].node, LPQ[i].message, 0);
        }
        MEM_(LPQ, 1, MESSAGE_container);
        LPQ_size = 0;
    }

exit:
    return ret;
}

int triggerExecute(uint8_t node, uint8_t trigger)
{
    int ret = SUCCESS;

    TRIGGER_container *temp;

    temp = lookup(node, TQ, TQ_size);
    if (temp == NULL) {
        ret = -1;
        goto exit;
    }

    if (get_bit((*temp).bitmap, trigger)) {
        demandInsert((*temp).node, (*temp).messages[trigger], 1);
        remove(&TQ, node, &TQ_size);
    } else {
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}
