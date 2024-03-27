#include "queue.h"
#include "utils.h"
#include <string.h>
#include "memory.h"
#include "headers.h"
#include "stack.h"

int i = 0;

int offsets;
void rawGetHeader(uint8_t *b)
{
    *b = rawBytes[offsets];
    offsets++;
    printf("Read: 1 Byte(s) as header\n");
}

void rawGetBytes(uint8_t **b, size_t n)
{
    for (uint32_t j = 0; j < n; j++) {
        *(*b + j) = rawBytes[offsets];
        offsets++;
    }

    printf("Read: %ld Byte(s) as raw\n", n);
}

void printNodeMap(void)
{
    printf("DQ:\n");
    for (unsigned int j = 0; j < DQ_stack.size; j++) {
        printf("\tNode %u: ", DQ_stack.data[j].node);
        for (unsigned int k = 0; k < DQ_stack.data[j].messageSize; k++) {
            printf("%1c", (char)(unsigned)DQ_stack.data[j].message[k]);
        }
        printf("\n");
    }

    printf("LPQ:\n");
    for (unsigned int j = 0; j < LPQ_stack.size; j++) {
        printf("\tNode %u: ", LPQ_stack.data[j].node);
        for (unsigned int k = 0; k < LPQ_stack.data[j].messageSize; k++) {
            printf("%1c", (char)(unsigned)LPQ_stack.data[j].message[k]);
        }
        printf("\n");
    }

    printf("TQ:\n");
    for (unsigned int j = 0; j < TQ_size; j++) {
        printf("\tNode %u\n", TQ[j].node);
        for (unsigned int b = 0; b < 64; b++) {
            if (get_bit(TQ[j].bitmap, b)) {
                printf("\t\tTrigger %u: ", b);
                for (unsigned int k = 0; k < TQ[j].messageSize[b]; k++) {
                    printf("%1c", (char)(unsigned)TQ[j].messages[b][k]);
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}

int main(void) {
    int ret = SUCCESS;

    while (i < 11) {
        ret = getMessage();
        CHECK(ret);
        i++;
        printNodeMap();
    }

exit:
    return ret;
}
