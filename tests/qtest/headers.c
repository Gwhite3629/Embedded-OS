#include "message.h"

// ASCII N 4E
// ASCII T 54
// ASCII 0 30
// ASCII M 4D
// ASCII L 4C
// ASCII D 44

/*
#define HEADER0 0x050201    // 0b000001010000001000000001, Trigger message N2T1
#define HEADER1 0x8303      // 0b1000001100000011, Low priority message N3
#define HEADER2 0x4304      // 0b0100001100000100, Demand message N4
#define HEADER3 0x050102    // 0b000001010000000100000010, Trigger message N1T2
#define HEADER4 0x050402    // 0b000001010000010000000010, Trigger message N4T2
#define HEADER5 0x050102    // 0b000001010000000100000010, Trigger message N1T2
#define HEADER6 0xC204      // 0b1100001000000100, Trigger N4T2
#define HEADER7 0xC102      // 0b1100000100000010, Trigger N2T1
#define HEADER8 0xC201      // 0b1100001000000001, Trigger N1T2
#define HEADER9 0x4302      // 0b0100001100000010, Demand message N2
#define HEADER10 0x8302      // 0b1000001100000010, Low priority message N2

#define MESSAGE0 0x4E3254314D    // N2T1M
#define MESSAGE1 0x4E334C        // N3L
#define MESSAGE2 0x4E3444        // N4D
#define MESSAGE3 0x4E3154324D    // N1T2M
#define MESSAGE4 0x4E3454324D    // N4T2M
#define MESSAGE5 0x4E3154324D    // N1T2M
#define MESSAGE6 0x0      
#define MESSAGE7 0x0      
#define MESSAGE8 0x0
#define MESSAGE9 0x4E3244        // N2D
#define MESSAGE10 0x4E324C        // N2L
*/

uint8_t rawBytes[58] = {
    0x05,0x02,0x01,0x4E,0x32,0x54,0x31,0x4D,
    0x83,0x03,0x4E,0x33,0x4C,
    0x43,0x04,0x4E,0x34,0x44,
    0x05,0x01,0x02,0x4E,0x31,0x54,0x32,0x4D,
    0x05,0x04,0x02,0x4E,0x34,0x54,0x32,0x4D,
    0x05,0x01,0x02,0x4E,0x31,0x54,0x32,0x4D,
    0xC2,0x04,
    0xC1,0x02,
    0xC2,0x01,
    0x43,0x02,0x4E,0x32,0x44,
    0x83,0x02,0x4E,0x32,0x4C,
};

int SIZES[22] = {
    3,5,
    2,3,
    2,3,
    3,5,
    3,5,
    3,5,
    2,0,
    2,0,
    2,0,
    2,3,
    2,3,
}; // Sizes of header,message