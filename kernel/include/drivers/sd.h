/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef EMMC_H_
#define EMMC_H_

#include "../stdlib/types.h"
#include "../stdlib/hardware_rpi4b.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct block_device
{
    char *driver_name;
    char *device_name;
    uint8_t *device_id;
    size_t dev_id_len;

    int supports_multiple_block_read;
    int supports_multiple_block_write;

    int (*read)(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_num);
    int (*write)(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_num);
    size_t block_size;
    size_t num_blocks;
};

struct emmc_block_dev
{
    struct block_device bd;
    uint32_t card_supports_sdhc;
    uint32_t card_supports_18v;
    uint32_t card_ocr;
    uint32_t card_dsr;
    uint16_t card_rca;
    uint64_t sd_hv;
    uint32_t last_interrupt;
    uint32_t last_error;

    uint32_t CID[4];
    uint32_t CSD[4];

    uint64_t scr[2];

    int failed_voltage_switch;

    uint32_t last_cmd_reg;
    uint32_t last_cmd;
    uint32_t last_cmd_success;
    uint32_t last_r0;
    uint32_t last_r1;
    uint32_t last_r2;
    uint32_t last_r3;

    void *buf;
    int blocks_to_transfer;
    size_t block_size;
    int use_sdma;
    int card_removal;
    uint32_t base_clock;
};

int sd_status(unsigned int mask);
int sd_int(unsigned int mask);
uint32_t sd_cmd(unsigned int code, unsigned int arg);
int sd_clk(unsigned int f);
void sd_gpio(void);

int sd_init(void);

size_t sd_read(uint8_t *buffer, size_t num, uint32_t lba);
size_t sd_write(uint8_t *buffer, size_t num, uint32_t lba);

extern struct emmc_block_dev *emmc_dev;

#endif // EMMC_H_