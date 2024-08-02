#include <stdlib/types.h>
#include <drivers/mailbox.h>
#include <stdlib/hardware_rpi4b.h>
#include <drivers/io.h>
#include <stdlib/printk.h>

volatile uint32_t __attribute__((aligned(16))) mbox[36];

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

void read_mailbox(uint32_t mbox)
{

}

void get_arm_address(void)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = ARM_MEMORY;
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
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                arm_base_address = mbox[5] + 0x800000;
                arm_size = mbox[6];
                printk("FINAL TAG:          %x\n", mbox[7]);
                return;
            }
        }
    }
}

void get_vc_address(void)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (0x8));

    mbox[0] = 4 * 8;
    mbox[1] = 0x00000000;
    mbox[2] = VC_MEMORY;
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
                printk("TAG IDENTIFIER:     %x\n", mbox[2]);
                printk("TAG V-BUFFER SIZE:  %x\n", mbox[3]);
                vc_base_address = mbox[5] + 0x800000;
                vc_size = mbox[6];
                printk("FINAL TAG:          %x\n", mbox[7]);
                return;
            }
        }
    }
}