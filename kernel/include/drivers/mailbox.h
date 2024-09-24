#ifndef _MAILBOX_H_
#define _MAILBOX_H_

#include <stdlib/types.h>
#include <drivers/graphics/framebuffer.h>

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

#define GET_POWER_STATE     0x00020001
#define GET_POWER_TIMING    0x00020002
#define SET_POWER_STATE     0x00028001

#define DMA_CHANNELS    0x00060001
extern uint32_t dma_mask;

// POWER ID
#define SD_POWER_ID         0x00000000
#define UART0_POWER_ID      0x00000001
#define UART1_POWER_ID      0x00000002
#define USB_HCD_POWER_ID    0x00000000
#define I2C0_POWER_ID       0x00000000
#define I2C1_POWER_ID       0x00000000
#define I2C2_POWER_ID       0x00000000
#define SPI_POWER_ID        0x00000000
#define CCP2TX_POWER_ID     0x00000000

// CLOCK ID
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

#define SET_CLOCK_RATE  0x00038002

/**************
 * GPU Memory *
 **************/

// Allocate contiguous memory on the GPU
#define ALLOCATE_MEMORY         0x0003000c
// Flags for allocation
#define MEM_FLAG_DISCARDABLE        (1 << 0)    /* can be resized to 0 at any time. Use for cached data */
#define MEM_FLAG_NORMAL             (0 << 2)    /* normal allocating alias. Don't use from ARM */
#define MEM_FLAG_DIRECT             (1 << 2)    /* 0xC alias uncached */
#define MEM_FLAG_COHERENT           (2 << 2)    /* 0x8 alias. Non-allocating in L2 but coherent */
#define MEM_FLAG_L1_NONALLOCATING   (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT), /* Allocating in L2 */
#define MEM_FLAG_ZERO               (1 << 4)    /* initialise buffer to all zeros */
#define MEM_FLAG_NO_INIT            (1 << 5)    /* don't initialise (default is initialise to all ones */
#define MEM_FLAG_HINT_PERMALOCK     (1 << 6)    /* Likely to be locked for long periods of time. */

// Lock allocated GPU memory
#define LOCK_MEMORY             0x0003000d
#define UNLOCK_MEMORY           0x0003000e

// Free GPU memory
#define RELEASE_MEMORY          0x0003000f

// Allocate framebuffer
#define ALLOCATE_FRAME_BUFFER   0x00040001
#define RELEASE_FRAME_BUFFER    0x00040002

#define BLANK_SCREEN            0x00040002

#define GET_DISPLAY             0x00040003
#define SET_DISPLAY             0x00048003

#define GET_VIRTUAL             0x00040004
#define SET_VIRTUAL             0x00048004

#define GET_DEPTH               0x00040005
#define SET_DEPTH               0x00048005

#define GET_PIXEL_ORDER         0x00040006
#define SET_PIXEL_ORDER         0x00048006

#define GET_ALPHA_MODE          0x00040007
#define SET_ALPHA_MODE          0x00048007

#define GET_PITCH               0x00040008

#define GET_VIRT_OFFSET         0x00040009
#define SET_VIRT_OFFSET         0x00048009

#define GET_OVERSCAN            0x0004000a
#define SET_OVERSCAN            0x0004800a

#define GET_PALETTE             0x0004000b
#define SET_PALETTE             0x0004800b

#define SET_CURSOR_INFO         0x00008010
#define SET_CURSOR_STATE        0x00008011

void get_arm_address(void);
void get_vc_address(void);

void setup_mbox(void);

int bcm_2708_power_off(void);
int bcm_2708_power_on(void);
int bcm_2708_get_state(void);

uint32_t sd_get_base_clock_hz(void);
uint32_t sd_set_base_clock_hz(uint32_t clock, uint32_t val1, uint32_t val2);

uint32_t allocate_GPU_memory(uint32_t size, uint32_t alignment, uint32_t flags);

uint32_t allocate_framebuffer(uint32_t alignment);
uint32_t release_framebuffer();

uint32_t blank_screen(uint32_t state);

uint32_t get_display_wh();
uint32_t set_display_wh(uint32_t width, uint32_t height);

uint32_t get_display_depth();
uint32_t set_display_depth(uint32_t depth);

uint32_t get_pitch();

uint32_t set_virt_wh(uint32_t virtWidth, uint32_t virtHeight);
uint32_t set_virt_offset(uint32_t x, uint32_t y);

uint32_t get_pixel_order();
uint32_t set_pixel_order(uint32_t state);

extern volatile uint32_t *mbox;

#endif // _MAILBOX_H_