#include "message.h"

// ASCII N 4E
// ASCII T 54
// ASCII 0 30
// ASCII M 4D
// ASCII L 4C
// ASCII D 44

message_t HEADERS[10] = {
    0b000001010000001000000001, // Trigger message N2T1
    0b1000001100000011, // Low priority message N3
    0b0100001100000100, // Demand message N4
    0b000001010000001000000010, // Trigger message N1T2
    0b000001010000010000000010, // Trigger message N2T2
    0b1100010000000001, // Trigger N1T2
    0b1100010000000010, // Trigger N2T2
    0b1100010000000001, // Trigger N2T1
    0b0100001100000010, // Demand message N2
    0b1000001100000010, // Low priority message N2
};

message_t MESSAGES[10] = {
    0x4E3254314D, // N2T1M
    0x4E334C, // N3L
    0x4E3444, // N4D
    0x4E3154324D, // N1T2M
    0x4E3254324D, // N2T2M
    0x4E315432, // N1T2
    0x4E325432, // N2T2
    0x4E325431, // N2T1
    0x4E3244, // N2D
    0x4E324C, // N2L
};

int SIZES[20] = {
    3,5,
    2,3,
    2,3,
    3,5,
    3,5,
    2,4,
    2,4,
    2,4,
    2,3,
    2,3,
}; // Sizes of header,message