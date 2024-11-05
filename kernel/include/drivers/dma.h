#ifndef _DMA_H_
#define _DMA_H_


// Generic control block structure
// Contains 8x32 bits
// Prefixes:
// D - DMA Control Block field
// L - DMA Lite Control Block field
// F - DMA4 Control Block field
typedef struct __attribute__((__packed__, aligned(32))) control_block {
    uint32_t    DLF_TI;
    uint32_t    DLF_SOURCE;
    union {
        uint32_t    DL_DEST;
        uint32_t    F_SRCI;
    };
    union {
        uint32_t    DL_TXFR_LEN;
        uint32_t    F_DEST;
    };
    union {
        uint32_t    D_STRIDE;
        uint32_t    L_RES;
        uint32_t    F_DESTI;
    };
    union {
        uint32_t    DL_NEXT_CB;
        uint32_t    F_TXFR_LEN;
    };
    union {
        uint32_t    DL_RES;
        uint32_t    F_NEXT_CB;
    };
    uint32_t    DLF_RES;
};

#endif // _DMA_H_
