#include "queue.h"
#include "utils.h"

int i = 0;

extern TRIGGER_array TQ;
extern uint32_t TQ_size;
extern MESSAGE_array DQ;
extern uint32_t DQ_size;
extern MESSAGE_array LPQ;
extern uint32_t LPQ_size;

extern message_t HEADERS[10];
extern message_t MESSAGES[10];
extern int SIZES[20];

message_t *rawBytes[10];

int offsets[10];
void rawGetHeader(uint8_t **b)
{
    *b = rawBytes[i][offsets[i]];
    offsets[i]++;
    *(b + 1) = rawBytes[i][offsets[i]];
    offsets[i]++;
}

void rawGetBytes(uint8_t *b, size_t n)
{
    for (int j = 0; j < n; j++) {
        *(b + j) = rawBytes[i][offsets[i]];
        offsets[i]++;
    }
}

int initRawBytes(void)
{
    int ret = SUCCESS;
    
    memset(offsets, 0, 10*sizeof(int));
    for (int j = 0; j < 10; j++) {
        int s = SIZES[2*j] + SIZES[2*(j+1)-1];
        MEM(rawBytes[j], s, uint8_t);
        memcpy(rawBytes[j], HEADERS[j], SIZES[2*j]);
        memcpy(rawBytes[j] + SIZES[2*j], MESSAGES[10], SIZES[2*(j+1)-1]);
    }

exit:
    return ret;
}

int main(void) {
    int ret = SUCCESS;

    ret = initRawBytes();

    while (i < 10) {
        ret = getMessage();
        CHECK(ret);
        i++;
    }

exit:
    return ret;
}
