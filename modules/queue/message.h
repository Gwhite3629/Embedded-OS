#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdint.h> // types.h
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "utils.h"

// First byte is always size
typedef uint8_t * message_t;

typedef struct MESSAGE_container {
    uint32_t node;
    message_t message;
};

typedef struct TRIGGER_container {
    uint32_t bitmap[2];
    message_t messages[64];
    uint8_t n_messages;
    uint32_t node;
};

typedef TRIGGER_container * TRIGGER_array;
typedef MESSAGE_container * MESSAGE_array;

static inline int cmp(const void *va, const void *vb)
{
    const TRIGGER_container *a = (const TRIGGER_container *)va;
    const TRIGGER_container *b = (const TRIGGER_container *)vb;
    return (int)(a->node - b->node);
}

static inline TRIGGER_container *
lookup(uint32_t node, TRIGGER_array table, uint32_t size)
{
    TRIGGER_container valid;
    valid.node = node;
    TRIGGER_container *r = (TRIGGER_container *)bsearch(&table, table, size, sizeof(TRIGGER_container), &cmp);
    return r ? r : NULL;
}

static inline int insert(TRIGGER_container **map, uint32_t node, uint32_t *size)
{
    int ret = 0;
    TRIGGER_container *temp = lookup(node, *map, (*size));
    if (temp != NULL) return 1;
    (*size)++;
    MEM_((*map), (*size), TRIGGER_container);
    (*map)[(*size)-1].node = node;
    qsort((*map), (*size), sizeof(TRIGGER_container), &cmp);

exit:
    return ret;
}

static inline int remove(TRIGGER_container **map, uint32_t node, uint32_t *size)
{
    int ret = SUCCESS;

    TRIGGER_container *temp = lookup(node, *map, (*size));
    TRIGGER_container *scan = (TRIGGER_container *)bsearch(&temp, (*map), (*size), sizeof(TRIGGER_container), &cmp);
    if (scan != NULL) {
        temp = &(*map)[(*size) - 1];
        (*map)[(*size) - 1] = (*scan);
        (*scan) = (*temp);
        (*size)--;
        MEM_((*map), (*size), TRIGGER_container);
    } else {
        printf("Node number not in map\n");
    }

exit:
    return ret;
}

/* 
 * TRIGGER MESSAGE
 *
 * 3 byte header:
 * |00| |00| |0000| <- First header
 * \__/ \__/ \____/
 *  ^    ^    ^
 *  |____|____|______ Indicates a trigger message
 *       |____|______ Indicates number of extra length headers
 *            |______ Indicates initial message size
 * 
 * |00000000|       <- Second header
 * \________/
 *  ^
 *  |________________ Indicates node number
 * 
 * |xx| |000000|    <- Third header
 * \__/ \______/
 *  ^    ^    
 *  |____|___________ Unused
 *       |___________ Indicates Trigger number
 *
 * All extra headers add to total message length.
 * Messages immediately follow extra headers.
 * 
 * Example:
 * 00011111 00010101 00000010 00001100 ...
 * Trigger message with 1 extra header,
 * Trigger number of 2,
 * Node number of 21,
 * Message size of 43 bytes.
 */

/*
 * TRIGGER
 *
 * 2 headers, no message
 * 
 * |11| |000000|    <- First header
 * \__/ \______/
 *  ^    ^
 *  |____|___________ Indicates trigger type
 *       |___________ Indicates trigger number
 * 
 * |00000000|       <- Second header
 * \________/
 *  ^
 *  |________________ Indicates node number
 * 
 * Example:
 * 11000010 00010101
 * Trigger type for number 2,
 * Node number of 21.
 */

/*
 * DEMAND MESSAGE
 *
 * 2 byte header:
 * |01| |00| |0000| <- First header
 * \__/ \__/ \____/
 *  ^    ^    ^
 *  |____|____|______ Indicates a demand message
 *       |____|______ Indicates number of extra length headers
 *            |______ Indicates initial message size
 * 
 * |00000000|       <- Second header
 * \________/
 *  ^
 *  |________________ Indicates node number
 *
 * All extra headers add to total message length.
 * Messages immediately follow extra headers.
 * 
 * Example:
 * 01011111 00010101 00001100 ...
 * Demand message with 1 extra header,
 * Node number of 21,
 * Message size of 43 bytes.
 */

/*
 * LOW PRIORITY MESSAGE
 *
 * 2 byte header:
 * |10| |00| |0000| <- First header
 * \__/ \__/ \____/
 *  ^    ^    ^
 *  |____|____|______ Indicates a low priority message
 *       |____|______ Indicates number of extra length headers
 *            |______ Indicates initial message size
 * 
 * |00000000|       <- Second header
 * \________/
 *  ^
 *  |________________ Indicates node number
 *
 * All extra headers add to total message length.
 * Messages immediately follow extra headers.
 * 
 * Example:
 * 10011111 00010101 00001100 ...
 * Low priority message with 1 extra header,
 * Node number of 21,
 * Message size of 43 bytes.
 * 
 */

#endif // _MESSAGE_H_