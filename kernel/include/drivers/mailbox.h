#ifndef _MAILBOX_H_
#define _MAILBOX_H_

#include <stdlib/types.h>

#define MAIL_FULL       0x80000000
#define MAIL_EMPTY      0x40000000

#define FIRMWARE_VER    0x00000001
extern uint32_t firmware_version;

#define BOARD_MODEL     0x00010001
extern uint32_t board_model;

#define BOARD_REV       0x00010002
extern uint32_t board_revision;

#define MAC_ADDRESS     0x00010003
extern uint8_t mac_address[6];

#define BOARD_SERIAL    0x00010004
extern uint64_t board_serial;

#define ARM_MEMORY      0x00010005
extern uint32_t arm_base_address;
extern uint32_t arm_size;

#define VC_MEMORY       0x00010006
extern uint32_t vc_base_address;
extern uint32_t vc_size;

#define GET_CLOCKS      0x00010007

#define DMA_CHANNELS    0x00060001
extern uint32_t dma_mask;

#define EMMC_CLOCK_ID       0x00000001
#define UART_CLOCK_ID       0x00000002
#define ARM_CLOCK_ID        0x00000003
#define CORE_CLOCK_ID       0x00000004
#define V3D_CLOCK_ID        0x00000005
#define H264_CLOCK_ID       0x00000006
#define ISP_CLOCK_ID        0x00000007
#define SDRAM_CLOCK_ID      0x00000008
#define PIXEL_CLOCK_ID      0x00000009
#define PWM_CLOCK_ID        0x0000000a
#define HEVC_CLOCK_ID       0x0000000b
#define EMMC2_CLOCK_ID      0x0000000c
#define M2MC_CLOCK_ID       0x0000000d
#define PIXEL_BVB_CLOCK_ID  0x0000000e

#define GET_CLOCK_STATE 0x00030001
extern uint32_t clock_id;
extern uint32_t clock_state;

#define SET_CLOCK_STATE 0x00038001

#define GET_CLOCK_RATE  0x00030002
extern uint32_t clock_id;
extern uint32_t clock_rate;

#define ALLOCATE_FRAME_BUFFER   0x00040001
extern uint32_t frame_buffer_base;
extern uint32_t frame_buffer_size;

#define RELEASE_FRAME_BUFFER    0x00040002

#define BLANK_SCREEN            0x00040002

#define GET_DISPLAY             0x00040003
extern uint32_t display_width;
extern uint32_t display_height;

#define SET_DISPLAY             0x00048003

#define GET_VIRTUAL             0x00040004
extern uint32_t virtual_width;
extern uint32_t virtual_height;

#define SET_VIRTUAL             0x00048004

#define GET_DEPTH               0x00040005
extern uint32_t bits_per_pixel;

#define SET_DEPTH               0x00048005

void read_mailbox(uint32_t mbox);

void get_arm_address(void);

void get_vc_address(void);

#endif // _MAILBOX_H_