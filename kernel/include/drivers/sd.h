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

static uint32_t big_to_little(uint32_t val)
{
    uint8_t tmp[4];
    tmp[0] = (val >> 24) & 0xf;
    tmp[1] = (val >> 16) & 0xf;
    tmp[2] = (val >> 8) & 0xf;
    tmp[3] = (val >> 0) & 0xf;

    return *(uint32_t *)tmp;
}

struct __attribute__((__packed__, aligned(4))) CID {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile uint8_t OID_Lo;
            volatile uint8_t OID_Hi;
            volatile uint8_t MID;
            unsigned reserved : 8;
        };
        volatile uint32_t raw32_0;
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile char prodName4 : 8;
            volatile char prodName3 : 8;
            volatile char prodName2 : 8;
            volatile char prodName1 : 8;
        };
        volatile uint32_t raw32_1;
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned serialNumHi : 16;
            volatile unsigned prodRevLo : 4;
            volatile unsigned prodRevHi : 4;
            volatile char prodName5 : 8;
        };
        volatile uint32_t raw32_2;
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned manMonth : 4;
            volatile unsigned manYear : 8;
            unsigned reserved1 : 4;
            volatile unsigned serialNumLo : 16;
        };
        volatile uint32_t raw32_3;
    };
};


struct __attribute__((__packed__, aligned(4))) CSD {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            enum {
                CSD_VERSION_1 = 0,
                CSD_VERSION_2 = 1,
            } csd_structure : 2;
            unsigned spec_vers : 6;
            uint8_t taac;
            uint8_t nsac;
            uint8_t tran_speed;
        };
        uint32_t raw32_0;
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            unsigned ccc : 12;
            unsigned read_bl_len : 4;
            unsigned read_bl_partial : 1;
            unsigned write_blk_misalign : 1;
            unsigned read_blk_misalign : 1;
            unsigned dsr_imp : 1;
            unsigned c_size : 12;
        };
        uint32_t raw32_1;
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            union {
                struct __attribute__((__packed__, aligned(1))) {
                    unsigned vdd_r_curr_min : 3;
                    unsigned vdd_r_curr_max : 3;
                    unsigned vdd_w_curr_min : 3;
                    unsigned vdd_w_curr_max : 3;
                    unsigned c_size_mult : 3;
                    unsigned reserved0 : 7;
                };
                unsigned ver2_c_size : 22;
            };
            unsigned erase_blk_en : 1;
            unsigned sector_size : 7;
            unsigned reserved1 : 2;
        };
        uint32_t raw32_2;
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            unsigned wp_grp_size : 7;
            unsigned wp_grp_enable : 1;
            unsigned reserved2 : 2;
            unsigned r2w_factor : 3;
            unsigned write_bl_len : 4;
            unsigned write_bl_partial : 1;
            unsigned default_ecc : 5;
            unsigned file_format_grp : 1;
            unsigned copy : 1;
            unsigned perm_write_protect : 1;
            unsigned tmp_write_protect : 1;
            enum {
                FAT_PARTITION_TABLE = 0,
                FAT_NO_PARTITION_TABLE = 1,
                FS_UNIVERSAL = 2,
                FS_OTHER = 3,
            } file_format : 2;
            unsigned ecc : 2;
            unsigned reserved3 : 1;
        };
        uint32_t raw32_3;
    };
};

struct __attribute__((__packed__, aligned(4))) SCR {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile enum {
                SD_SPEC_1_101 = 0,
                SD_SPEC_11 = 1,
                SD_SPEC_2_3 = 2,
            } SD_SPEC : 4;
            volatile enum {
                SCR_VER_1 = 0,
            } SCR_STRUCT : 4;
            volatile enum {
                BUS_WIDTH_1 = 1,
                BUS_WIDTH_4 = 4,
            } BUS_WIDTH : 4;
            volatile enum {
                SD_SEC_NONE = 0,
                SD_SEC_NOT_USED = 1,
                SD_SEC_101 = 2,
                SD_SEC_2 = 3,
                SD_SEC_3 = 4,
            } SD_SECURITY : 3;
            volatile unsigned DATA_AFTER_ERASE : 1;
            unsigned reserved : 3;
            volatile enum {
                EX_SEC_NONE = 0,
            } EX_SECURITY : 4;
            volatile unsigned SD_SPEC3 : 1;
            volatile enum {
                CMD_SUPP_SPEED_CLASS = 1,
                CMD_SUPP_SET_BLKCNT = 2,
            } CMD_SUPPORT : 2;
            unsigned reserved1 : 6;
        };
        volatile uint32_t raw32_lo;
    };
    volatile uint32_t raw32_hi;
};

/*
typedef union __attribute__((packed)) {
    struct __attribute__((packed)) CID_fields {
        unsigned NA:1;      // Always 1
        unsigned CRC:7;     // CRC7 checksum
        unsigned MDT:12;            // 2x4 hex digits month, year manufacturing date
        unsigned RES:4;
        unsigned PSN:32;    // A 32 bit unsigned binary integer
        unsigned PRV:8;           // 2x4 bit BCD revision number
        char PNM[5];    // 5 ASCII character product name
        unsigned OID:16;     // Card OEM Identification
        unsigned MID:8;     // Manufacturer Identification
    } fields;
    uint32_t val[4];
} CID;

typedef union __attribute__((packed)) {
    struct __attribute__((packed)) CSD_FIELDS {
        unsigned NA:1;
        unsigned CRC:7;
        unsigned ECC:2;
        unsigned FILE_FORMAT:2;
        unsigned TMP_WRITE_PROTECT:1;
        unsigned PERM_WRITE_PROTECT:1;
        unsigned COPY:1;
        unsigned FILE_FORMAT_GRP:1;
        unsigned CONTENT_PROT_APP:1;
        unsigned RES1:4;
        unsigned WRITE_BL_PARTIAL:1;
        unsigned WRITE_BL_LEN:4;
        unsigned R2W_FACTER:3;
        unsigned DEFAULT_ECC:2;
        unsigned WP_GRP_ENABLE:1;
        unsigned WP_GRP_SIZE:5;
        unsigned ERASE_GRP_MULT:5;
        unsigned ERASE_GRP_SIZE:5;
        unsigned C_SIZE_MULT:3;
        unsigned VDD_W_CURR_MAX:3;
        unsigned VDD_W_CURR_MIN:3;
        unsigned VDD_R_CURR_MAX:3;
        unsigned VDD_R_CURR_MIN:3;
        unsigned C_SIZE:12;
        unsigned RES2:2;
        unsigned DSR_IMP:1;
        unsigned READ_BLK_MISALIGN:1;
        unsigned WRITE_BLK_MISALIGN:1;
        unsigned READ_BL_PARTIAL:1;
        unsigned READ_BL_LEN:4;
        unsigned CCC:12;
        unsigned TRAN_SPEED:8;
        unsigned NSAC:8;
        unsigned TAAC:8;
        unsigned RES3:2;
        unsigned SPEC_VERS:4;
        unsigned CSD_STRUCTURE:2;
    } fields;
    uint32_t val[4];
} CSD;
*/
struct block_device
{
    char *driver_name;
    char *device_name;
    uint8_t *device_id;
    size_t dev_id_len;

    int supports_multiple_block_read;
    int supports_multiple_block_write;

    size_t (*read)(uint8_t *buf, size_t buf_size, uint32_t block_num);
    size_t (*write)(uint8_t *buf, size_t buf_size, uint32_t block_num);
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

    //uint32_t CID[4];
    //uint32_t CSD[4];

    struct CID CID;
    struct CSD CSD;
    
    uint64_t capacity;

    struct SCR SCR;

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

void sd_reset(void);
int sd_init(void);

size_t sd_read(uint8_t *buffer, size_t num, uint32_t lba);
size_t sd_write(uint8_t *buffer, size_t num, uint32_t lba);

void print_cid(void);
void print_csd(void);

extern struct emmc_block_dev *emmc_dev;

#endif // EMMC_H_
