#include <stdlib/types.h>
#include <drivers/mailbox.h>
#include <stdlib/hardware_rpi4b.h>
#include <drivers/io.h>
#include <stdlib/printk.h>

volatile uint32_t __attribute__((aligned(16))) early_mbox[36];
volatile uint32_t *mbox = early_mbox;

uint32_t firmware_version;
uint32_t board_model;
uint32_t board_revision;
uint8_t mac_address[6];
uint64_t board_serial;
uint32_t arm_base_address;
uint32_t arm_size;
uint32_t vc_base_address;
uint32_t vc_size;
uint32_t dma_mask;
uint32_t clock_id;
uint32_t clock_state;
uint32_t clock_id;
uint32_t clock_rate;
uint32_t frame_buffer_base;
uint32_t frame_buffer_size;
uint32_t display_width;
uint32_t display_height;
uint32_t virtual_width;
uint32_t virtual_height;
uint32_t bits_per_pixel;

void get_arm_address(void)
{
    unsigned int r = (((unsigned int)((unsigned long)&early_mbox)&~0xF) | (0x8));

    early_mbox[0] = 4 * 8;
    early_mbox[1] = 0x00000000;
    early_mbox[2] = ARM_MEMORY;
    early_mbox[3] = 0x00000008;
    early_mbox[4] = 0x00000008;
    early_mbox[5] = 0x00000000;
    early_mbox[6] = 0x00000000;
    early_mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (early_mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", early_mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", early_mbox[3]);
                arm_base_address = early_mbox[5] + 0x100000;
                arm_size = early_mbox[6];
                printk("FINAL TAG:          %x\n", early_mbox[7]);
                return;
            }
        }
    }
}

void get_vc_address(void)
{
    unsigned int r = (((unsigned int)((unsigned long)&early_mbox)&~0xF) | (0x8));

    early_mbox[0] = 4 * 8;
    early_mbox[1] = 0x00000000;
    early_mbox[2] = VC_MEMORY;
    early_mbox[3] = 0x00000008;
    early_mbox[4] = 0x00000008;
    early_mbox[5] = 0x00000000;
    early_mbox[6] = 0x00000000;
    early_mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (early_mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", early_mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", early_mbox[3]);
                vc_base_address = early_mbox[5] + 0x800000;
                vc_size = early_mbox[6];
                printk("FINAL TAG:          %x\n", early_mbox[7]);
                return;
            }
        }
    }
}

int bcm_2708_power_off(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_POWER_STATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000002;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("POWER DEVICE ID:    %x\n", mbox[5]);
                printk("POWER DEVICE STATE: %x\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[7]);
                return 0;
            }
        }
    }

    return -1;
}

int bcm_2708_power_on(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_POWER_STATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000003;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("POWER DEVICE ID:    %x\n", mbox[5]);
                printk("POWER DEVICE STATE: %x\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[7]);
                return 0;
            }
        }
    }

    return -1;
}

int bcm_2708_get_state(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = GET_POWER_STATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);
    printk("MAIL DONE WAITING 1\n");

    chip_write(r, MAIL_WRITE);
    printk("MAIL WRITTEN\n");

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);
        printk("MAIL DONE WAITING 2\n");

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("POWER DEVICE ID:    %x\n", mbox[5]);
                printk("POWER DEVICE STATE: %x\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[7]);
                return 0;
            }
        }
    }

    return -1;
}

uint32_t sd_get_base_clock_hz(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = GET_CLOCK_RATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000004;
    mbox[5] = EMMC_CLOCK_ID;
    mbox[6] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("CLOCK ID:           %x\n", mbox[5]);
                printk("CLOCK RATE:        %d\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[7]);
                return mbox[6];
            }
        }
    }

    return -1;
}

uint32_t sd_set_base_clock_hz(uint32_t clock, uint32_t val1, uint32_t val2)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 9;
    mbox[1] = 0x00000000;
    mbox[2] = SET_CLOCK_RATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = clock;
    mbox[6] = val1;
    mbox[7] = val2;
    mbox[8] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("CLOCK ID:           %x\n", mbox[5]);
                printk("CLOCK RATE:         %x\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[8]);
                return mbox[6];
            }
        }
    }

    return -1;
}

uint32_t sd_get_clock_state(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = GET_CLOCK_STATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000004;
    mbox[5] = EMMC_CLOCK_ID;
    mbox[6] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("CLOCK ID:           %x\n", mbox[5]);
                printk("CLOCK STATE:        %d\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[7]);
                return mbox[6];
            }
        }
    }

    return -1;
}

uint32_t sd_set_clock_state(uint32_t state)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_CLOCK_STATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = EMMC_CLOCK_ID;
    mbox[6] = state;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("CLOCK ID:           %x\n", mbox[5]);
                printk("CLOCK STATE:        %d\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[7]);
                return mbox[6];
            }
        }
    }

    return -1;
}

uint32_t allocate_GPU_memory(uint32_t size, uint32_t alignment, uint32_t flags)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 9;
    mbox[1] = 0x00000000;
    mbox[2] = ALLOCATE_MEMORY;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = size;
    mbox[6] = alignment;
    mbox[7] = flags;
    mbox[8] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                return mbox[5];
            }
        }
    }

    return -1;
}

uint32_t allocate_framebuffer(uint32_t alignment)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = ALLOCATE_FRAME_BUFFER;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = alignment;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("\x1b[1;32mFRAMEBUFFER ALLOCATED\x1b[1;0m\n");
                fb.buf = (unsigned char *)(uint64_t)(mbox[5] & 0x3FFFFFFF);
                fb.buf_size = mbox[6];
                printk("\tADDR: %x SIZE: %x\n", fb.buf, fb.buf_size);
                return 0;
            }
        }
    }

    return -1;
}

uint32_t release_framebuffer(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = RELEASE_FRAME_BUFFER;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                fb.buf = 0;
                fb.buf_size = 0;
                return 0;
            }
        }
    }

    return -1;
}

uint32_t blank_screen(uint32_t state)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = BLANK_SCREEN;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = state;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                return 0;
            }
        }
    }

    return -1;
}

uint32_t get_display_wh(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = GET_DISPLAY;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("\x1b[1;32mGOT FRAME STATS\x1b[1;0m\n");
                printk("\tWIDTH: %d HEIGHT: %d\n", mbox[5], mbox[6]);
                fb.width = mbox[5];
                fb.height = mbox[6];
                return 0;
            }
        }
    }

    return -1;
}

uint32_t set_display_wh(uint32_t width, uint32_t height)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_DISPLAY;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000000;
    mbox[5] = width;
    mbox[6] = height;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("\x1b[1;32mSET FRAME STATS\x1b[1;0m\n");
                printk("\tWIDTH: %d HEIGHT: %d\n", mbox[5], mbox[6]);
                fb.width = mbox[5];
                fb.height = mbox[6];
                if ((width == mbox[5]) & (height == mbox[6])) {
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }

    return -1;
}

uint32_t get_display_depth(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = GET_DEPTH;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                fb.depth = mbox[5];
            }
        }
    }

    return -1;
}

uint32_t set_display_depth(uint32_t depth)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_DEPTH;
    mbox[3] = 0x00000004;
    mbox[4] = 0x00000004;
    mbox[5] = depth;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                fb.depth = mbox[5];
                if (depth == mbox[5]) {
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }

    return -1;
}

uint32_t get_pitch(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = GET_PITCH;
    mbox[3] = 0x00000004;
    mbox[4] = 0x00000004;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("\x1b[1;32mGOT PITCH\x1b[1;0m\n");
                printk("\tPITCH: %x\n", mbox[5]);
                fb.pitch = mbox[5];
                return 0;
            }
        }
    }

    return -1;
}

uint32_t set_virt_wh(uint32_t virtWidth, uint32_t virtHeight)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_VIRTUAL;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = virtWidth;
    mbox[6] = virtHeight;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("\x1b[1;32mSET VIRT FRAME STATS\x1b[1;0m\n");
                printk("\tWIDTH: %d HEIGHT: %d\n", mbox[5], mbox[6]);
                fb.virtWidth = mbox[5];
                fb.virtHeight = mbox[6];
                if ((virtWidth == mbox[5]) & (virtHeight == mbox[6])) {
                    return 0;
                } else {
                    return 1;
                }
                return 0;
            }
        }
    }

    return -1;
}

uint32_t set_virt_offset(uint32_t x, uint32_t y)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_VIRT_OFFSET;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = x;
    mbox[6] = y;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                return 0;
            }
        }
    }

    return -1;
}

uint32_t get_pixel_order(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = GET_PIXEL_ORDER;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                fb.pixel_order = mbox[5];
                return 0;
            }
        }
    }

    return -1;
}

uint32_t set_pixel_order( uint32_t state)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_PIXEL_ORDER;
    mbox[3] = 0x00000004;
    mbox[4] = 0x00000004;
    mbox[5] = state;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                fb.pixel_order = mbox[5];
                if (state == mbox[5]) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }

    return -1;
}

void sd_disable_low_power(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = 0x00038041;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000084;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                printk("GPIO ID:            %x\n", mbox[5]);
                printk("GPIO STATE:         %d\n", mbox[6]);
                printk("FINAL TAG:          %x\n", mbox[7]);
                return;
            }
        }
    }

    return;
}

void sd_power_off(void)
{
    unsigned int r = (((unsigned int )((unsigned long)mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = SET_POWER_STATE;
    mbox[3] = 0x00000008;
    mbox[4] = 0x00000008;
    mbox[5] = 0x00000000;
    mbox[6] = 0x00000000;
    mbox[7] = 0x00000000;

    do {
        asm volatile("nop");
    } while(chip_read(MAIL_STAT) & MAIL_FULL);

    chip_write(r, MAIL_WRITE);

    while (1) {
        do {
            asm volatile("nop");
        } while(chip_read(MAIL_STAT) & MAIL_EMPTY);

        if (r == chip_read(MAIL_READ)) {
            if (mbox[1] == 0x80000000) {
                return;
            }
        }
    }
}
