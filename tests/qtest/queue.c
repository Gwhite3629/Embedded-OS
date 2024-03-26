#include "message.h"
#include "bitmap.h"
#include "queue.h"
#include "memory.h"

TRIGGER_array TQ;
uint32_t TQ_size;
MESSAGE_array DQ;
uint32_t DQ_size;
MESSAGE_array LPQ;
uint32_t LPQ_size;

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
        //triggerInsert(nodeNumber, triggerNumber, message);
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
        }
        goto exit;
    }

    CHECK(insert(&TQ, node, &TQ_size));
    container = lookup(node, TQ, TQ_size);
    (*container).bitmap[0] = 0;
    (*container).bitmap[1] = 0;
    memset((*container).messages, 0, 64*sizeof(message_t));
    set_bit((*container).bitmap, trigger);
    (*container).node = node;
    new(TQ[0].messages[trigger], messageSize, uint8_t);
    memcpy((*container).messages[trigger], message, messageSize);
    (*container).messageSize[trigger] = messageSize;

exit:
    return ret;
}

int demandInsert(uint8_t node, uint8_t *message, bool tb, uint32_t messageSize)
{
    printf("Found: Demand\n");
    int ret = SUCCESS;

    if ((DQ_size == 0) && (DQ == NULL)) {
        DQ_size = 1;
        new(DQ, 1, MESSAGE_container);
        new(DQ[0].message, messageSize, uint8_t);
        memcpy(DQ[0].message, message, messageSize);
        DQ[0].messageSize = messageSize;
        DQ[0].node = node;
        goto exit;
    } else if ((DQ_size == 0) && (DQ != NULL)) {
        DQ_size = 1;
        
        new(DQ[0].message, messageSize, uint8_t);
        memcpy(DQ[0].message, message, messageSize);
        DQ[0].messageSize = messageSize;
        DQ[0].node = node;
        goto exit;
    } else {
        DQ_size++;
        alt(DQ, DQ_size, MESSAGE_container);
    }

    if (tb) {
        for (unsigned int i = DQ_size; i > 1; i--) {
            DQ[i-1].message = DQ[i-2].message;
            DQ[i-1].messageSize = DQ[i-2].messageSize;
            DQ[i-1].node = DQ[i-2].node;
        }
        new(DQ[0].message, messageSize, uint8_t);
        memcpy(DQ[0].message, message, messageSize);
        DQ[0].messageSize = messageSize;
        DQ[0].node = node;
    } else {
        new(DQ[DQ_size-1].message, messageSize, uint8_t);
        memcpy(DQ[DQ_size-1].message, message, messageSize);
        DQ[DQ_size-1].messageSize = messageSize;
        DQ[DQ_size-1].node = node;
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

    if ((LPQ_size == 0) && (LPQ == NULL)) {
        LPQ_size = 1;
        new(LPQ, 1, MESSAGE_container);
        new(LPQ[0].message, messageSize, uint8_t);
        memcpy(LPQ[0].message, message, messageSize);
        LPQ[0].messageSize = messageSize;
        LPQ[0].node = node;
        goto exit;
    } else if ((LPQ_size == 0) && (LPQ != NULL)) {
        LPQ_size = 1;
        new(LPQ[0].message, messageSize, uint8_t);
        memcpy(LPQ[0].message, message, messageSize);
        LPQ[0].messageSize = messageSize;
        LPQ[0].node = node;
        goto exit;
    } else {
        LPQ_size++;
        alt(LPQ, LPQ_size, MESSAGE_container);
        new(LPQ[LPQ_size-1].message, messageSize, uint8_t);
        memcpy(LPQ[LPQ_size-1].message, message, messageSize);
        LPQ[LPQ_size-1].messageSize = messageSize;
        LPQ[LPQ_size-1].node = node;
    }

    // Promote all if DQ is empty
    if (DQ_size == 0) {
        for (uint32_t i = 0; i < LPQ_size; i++) {
            demandInsert(LPQ[i].node, LPQ[i].message, 0, messageSize);
        }
        alt(LPQ, 1, MESSAGE_container);
        LPQ_size = 0;
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
