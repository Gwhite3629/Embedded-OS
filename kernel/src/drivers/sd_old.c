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

/* Provides an interface to the EMMC controller and commands for interacting
 * with an sd card */

/* References:
 *
 * PLSS 	- SD Group Physical Layer Simplified Specification ver 3.00
 * HCSS		- SD Group Host Controller Simplified Specification ver 3.00
 *
 * Broadcom BCM2835 Peripherals Guide
 */

#include <stdlib.h>

#include <drivers/sd.h>
#include <drivers/platform.h>
#include <drivers/mailbox.h>

#include <memory/malloc.h>

#define EMMC_DEBUG

// Configuration options

// Enable 1.8V support
//#define SD_1_8V_SUPPORT

// Enable 4-bit support
//#define SD_4BIT_DATA

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID         400000
#define SD_CLOCK_NORMAL     25000000
#define SD_CLOCK_HIGH       50000000
#define SD_CLOCK_100        100000000
#define SD_CLOCK_208        208000000

// Enable SDXC maximum performance mode
// Requires 150 mA power so disabled on the RPi for now
//#define SDXC_MAXIMUM_PERFORMANCE

// Enable SDMA support
//#define SDMA_SUPPORT

// SDMA buffer address
#define SDMA_BUFFER     0x6000
#define SDMA_BUFFER_PA  (SDMA_BUFFER + 0xC0000000)

// Enable card interrupts
//#define SD_CARD_INTERRUPTS

// Enable EXPERIMENTAL (and possibly DANGEROUS) SD write support
#define SD_WRITE_SUPPORT 1

// The particular SDHCI implementation
#define SDHCI_IMPLEMENTATION_GENERIC        0
#define SDHCI_IMPLEMENTATION_BCM_2708       1
#define SDHCI_IMPLEMENTATION                SDHCI_IMPLEMENTATION_BCM_2708

static char driver_name[] = "emmc";
static char device_name[] = "emmc0";	// We use a single device name as there is only
					// one card slot in the RPi

static uint32_t hci_ver = 0;
static uint32_t capabilities_0 = 0;
static uint32_t capabilities_1 = 0;

struct sd_scr
{
    uint32_t    scr[2];
    uint32_t    sd_bus_widths;
    int         sd_version;
};

struct emmc_block_dev emmc_dev;

//#define EMMC_BASE		(IO_BASE + 0x00340000) // EMMC2 (new method(doesn't work))
#define EMMC_BASE		(IO_BASE + 0x00300000)
#define EMMC_ARG2		    (0x00)
#define EMMC_BLKSIZECNT		(0x04)
#define EMMC_ARG1		    (0x08)
#define EMMC_CMDTM		    (0x0C)
#define EMMC_RESP0		    (0x10)
#define EMMC_RESP1		    (0x14)
#define EMMC_RESP2		    (0x18)
#define EMMC_RESP3		    (0x1C)
#define EMMC_DATA		    (0x20)
#define EMMC_STATUS		    (0x24)
#define EMMC_CONTROL0		(0x28)
#define EMMC_CONTROL1		(0x2C)
#define EMMC_INTERRUPT		(0x30)
#define EMMC_IRPT_MASK		(0x34)
#define EMMC_IRPT_EN		(0x38)
#define EMMC_CONTROL2		(0x3C)
#define EMMC_CAPABILITIES_0	(0x40)
#define EMMC_CAPABILITIES_1	(0x44)
#define EMMC_FORCE_IRPT		(0x50)
#define EMMC_BOOT_TIMEOUT	(0x70)
#define EMMC_DBG_SEL		(0x74)
#define EMMC_EXRDFIFO_CFG	(0x80)
#define EMMC_EXRDFIFO_EN	(0x84)
#define EMMC_TUNE_STEP		(0x88)
#define EMMC_TUNE_STEPS_STD	(0x8C)
#define EMMC_TUNE_STEPS_DDR	(0x90)
#define EMMC_SPI_INT_SPT	(0xF0)
#define EMMC_SLOTISR_VER	(0xFC)

#define SD_CMD_INDEX(a)		((a) << 24)
#define SD_CMD_TYPE_NORMAL	0x0
#define SD_CMD_TYPE_SUSPEND	(1 << 22)
#define SD_CMD_TYPE_RESUME	(2 << 22)
#define SD_CMD_TYPE_ABORT	(3 << 22)
#define SD_CMD_TYPE_MASK    (3 << 22)
#define SD_CMD_ISDATA		(1 << 21)
#define SD_CMD_IXCHK_EN		(1 << 20)
#define SD_CMD_CRCCHK_EN	(1 << 19)
#define SD_CMD_RSPNS_TYPE_NONE	0			// For no response
#define SD_CMD_RSPNS_TYPE_136	(1 << 16)		// For response R2 (with CRC), R3,4 (no CRC)
#define SD_CMD_RSPNS_TYPE_48	(2 << 16)		// For responses R1, R5, R6, R7 (with CRC)
#define SD_CMD_RSPNS_TYPE_48B	(3 << 16)		// For responses R1b, R5b (with CRC)
#define SD_CMD_RSPNS_TYPE_MASK  (3 << 16)
#define SD_CMD_MULTI_BLOCK	(1 << 5)
#define SD_CMD_DAT_DIR_HC	0
#define SD_CMD_DAT_DIR_CH	(1 << 4)
#define SD_CMD_AUTO_CMD_EN_NONE	0
#define SD_CMD_AUTO_CMD_EN_CMD12	(1 << 2)
#define SD_CMD_AUTO_CMD_EN_CMD23	(2 << 2)
#define SD_CMD_BLKCNT_EN		(1 << 1)
#define SD_CMD_DMA          1

#define SD_ERR_CMD_TIMEOUT	0
#define SD_ERR_CMD_CRC		1
#define SD_ERR_CMD_END_BIT	2
#define SD_ERR_CMD_INDEX	3
#define SD_ERR_DATA_TIMEOUT	4
#define SD_ERR_DATA_CRC		5
#define SD_ERR_DATA_END_BIT	6
#define SD_ERR_CURRENT_LIMIT	7
#define SD_ERR_AUTO_CMD12	8
#define SD_ERR_ADMA		9
#define SD_ERR_TUNING		10
#define SD_ERR_RSVD		11

#define SD_ERR_MASK_CMD_TIMEOUT		(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_CMD_CRC		(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_CMD_END_BIT		(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CMD_INDEX		(1 << (16 + SD_ERR_CMD_INDEX))
#define SD_ERR_MASK_DATA_TIMEOUT	(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_DATA_CRC		(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_DATA_END_BIT	(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CURRENT_LIMIT	(1 << (16 + SD_ERR_CMD_CURRENT_LIMIT))
#define SD_ERR_MASK_AUTO_CMD12		(1 << (16 + SD_ERR_CMD_AUTO_CMD12))
#define SD_ERR_MASK_ADMA		(1 << (16 + SD_ERR_CMD_ADMA))
#define SD_ERR_MASK_TUNING		(1 << (16 + SD_ERR_CMD_TUNING))

#define SD_COMMAND_COMPLETE     1
#define SD_TRANSFER_COMPLETE    (1 << 1)
#define SD_BLOCK_GAP_EVENT      (1 << 2)
#define SD_DMA_INTERRUPT        (1 << 3)
#define SD_BUFFER_WRITE_READY   (1 << 4)
#define SD_BUFFER_READ_READY    (1 << 5)
#define SD_CARD_INSERTION       (1 << 6)
#define SD_CARD_REMOVAL         (1 << 7)
#define SD_CARD_INTERRUPT       (1 << 8)

#define SD_RESP_NONE        SD_CMD_RSPNS_TYPE_NONE
#define SD_RESP_R1          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R1b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R2          (SD_CMD_RSPNS_TYPE_136 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R3          SD_CMD_RSPNS_TYPE_48
#define SD_RESP_R4          SD_CMD_RSPNS_TYPE_136
#define SD_RESP_R5          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R5b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R6          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R7          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)

#define SD_DATA_READ        (SD_CMD_ISDATA | SD_CMD_DAT_DIR_CH)
#define SD_DATA_WRITE       (SD_CMD_ISDATA | SD_CMD_DAT_DIR_HC)

#define SD_CMD_RESERVED(a)  0xffffffff

#define SUCCESS(a)          (a->last_cmd_success)
#define FAIL(a)             (a->last_cmd_success == 0)
#define TIMEOUT(a)          (FAIL(a) && (a->last_error == 0))
#define CMD_TIMEOUT(a)      (FAIL(a) && (a->last_error & (1 << 16)))
#define CMD_CRC(a)          (FAIL(a) && (a->last_error & (1 << 17)))
#define CMD_END_BIT(a)      (FAIL(a) && (a->last_error & (1 << 18)))
#define CMD_INDEX(a)        (FAIL(a) && (a->last_error & (1 << 19)))
#define DATA_TIMEOUT(a)     (FAIL(a) && (a->last_error & (1 << 20)))
#define DATA_CRC(a)         (FAIL(a) && (a->last_error & (1 << 21)))
#define DATA_END_BIT(a)     (FAIL(a) && (a->last_error & (1 << 22)))
#define CURRENT_LIMIT(a)    (FAIL(a) && (a->last_error & (1 << 23)))
#define ACMD12_ERROR(a)     (FAIL(a) && (a->last_error & (1 << 24)))
#define ADMA_ERROR(a)       (FAIL(a) && (a->last_error & (1 << 25)))
#define TUNING_ERROR(a)     (FAIL(a) && (a->last_error & (1 << 26)))

#define SD_VER_UNKNOWN      0
#define SD_VER_1            1
#define SD_VER_1_1          2
#define SD_VER_2            3
#define SD_VER_3            4
#define SD_VER_4            5

static char *sd_versions[] = { "unknown", "1.0 and 1.01", "1.10",
    "2.00", "3.0x", "4.xx" };

#ifdef EMMC_DEBUG
static char *err_irpts[] = { "CMD_TIMEOUT", "CMD_CRC", "CMD_END_BIT", "CMD_INDEX",
	"DATA_TIMEOUT", "DATA_CRC", "DATA_END_BIT", "CURRENT_LIMIT",
	"AUTO_CMD12", "ADMA", "TUNING", "RSVD" };
#endif

static uint32_t sd_commands[] = {
    SD_CMD_INDEX(0),
    SD_CMD_RESERVED(1),
    SD_CMD_INDEX(2) | SD_RESP_R2,
    SD_CMD_INDEX(3) | SD_RESP_R6,
    SD_CMD_INDEX(4),
    SD_CMD_INDEX(5) | SD_RESP_R4,
    SD_CMD_INDEX(6) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(7) | SD_RESP_R1b,
    SD_CMD_INDEX(8) | SD_RESP_R7,
    SD_CMD_INDEX(9) | SD_RESP_R2,
    SD_CMD_INDEX(10) | SD_RESP_R2,
    SD_CMD_INDEX(11) | SD_RESP_R1,
    SD_CMD_INDEX(12) | SD_RESP_R1b | SD_CMD_TYPE_ABORT,
    SD_CMD_INDEX(13) | SD_RESP_R1,
    SD_CMD_RESERVED(14),
    SD_CMD_INDEX(15),
    SD_CMD_INDEX(16) | SD_RESP_R1,
    SD_CMD_INDEX(17) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(18) | SD_RESP_R1 | SD_DATA_READ | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN,
    SD_CMD_INDEX(19) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(20) | SD_RESP_R1b,
    SD_CMD_RESERVED(21),
    SD_CMD_RESERVED(22),
    SD_CMD_INDEX(23) | SD_RESP_R1,
    SD_CMD_INDEX(24) | SD_RESP_R1 | SD_DATA_WRITE,
    SD_CMD_INDEX(25) | SD_RESP_R1 | SD_DATA_WRITE | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN,
    SD_CMD_RESERVED(26),
    SD_CMD_INDEX(27) | SD_RESP_R1 | SD_DATA_WRITE,
    SD_CMD_INDEX(28) | SD_RESP_R1b,
    SD_CMD_INDEX(29) | SD_RESP_R1b,
    SD_CMD_INDEX(30) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_RESERVED(31),
    SD_CMD_INDEX(32) | SD_RESP_R1,
    SD_CMD_INDEX(33) | SD_RESP_R1,
    SD_CMD_RESERVED(34),
    SD_CMD_RESERVED(35),
    SD_CMD_RESERVED(36),
    SD_CMD_RESERVED(37),
    SD_CMD_INDEX(38) | SD_RESP_R1b,
    SD_CMD_RESERVED(39),
    SD_CMD_RESERVED(40),
    SD_CMD_RESERVED(41),
    SD_CMD_RESERVED(42) | SD_RESP_R1,
    SD_CMD_RESERVED(43),
    SD_CMD_RESERVED(44),
    SD_CMD_RESERVED(45),
    SD_CMD_RESERVED(46),
    SD_CMD_RESERVED(47),
    SD_CMD_RESERVED(48),
    SD_CMD_RESERVED(49),
    SD_CMD_RESERVED(50),
    SD_CMD_RESERVED(51),
    SD_CMD_RESERVED(52),
    SD_CMD_RESERVED(53),
    SD_CMD_RESERVED(54),
    SD_CMD_INDEX(55) | SD_RESP_R1,
    SD_CMD_INDEX(56) | SD_RESP_R1 | SD_CMD_ISDATA,
    SD_CMD_RESERVED(57),
    SD_CMD_RESERVED(58),
    SD_CMD_RESERVED(59),
    SD_CMD_RESERVED(60),
    SD_CMD_RESERVED(61),
    SD_CMD_RESERVED(62),
    SD_CMD_RESERVED(63)
};

static uint32_t sd_acommands[] = {
    SD_CMD_RESERVED(0),
    SD_CMD_RESERVED(1),
    SD_CMD_RESERVED(2),
    SD_CMD_RESERVED(3),
    SD_CMD_RESERVED(4),
    SD_CMD_RESERVED(5),
    SD_CMD_INDEX(6) | SD_RESP_R1,
    SD_CMD_RESERVED(7),
    SD_CMD_RESERVED(8),
    SD_CMD_RESERVED(9),
    SD_CMD_RESERVED(10),
    SD_CMD_RESERVED(11),
    SD_CMD_RESERVED(12),
    SD_CMD_INDEX(13) | SD_RESP_R1,
    SD_CMD_RESERVED(14),
    SD_CMD_RESERVED(15),
    SD_CMD_RESERVED(16),
    SD_CMD_RESERVED(17),
    SD_CMD_RESERVED(18),
    SD_CMD_RESERVED(19),
    SD_CMD_RESERVED(20),
    SD_CMD_RESERVED(21),
    SD_CMD_INDEX(22) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(23) | SD_RESP_R1,
    SD_CMD_RESERVED(24),
    SD_CMD_RESERVED(25),
    SD_CMD_RESERVED(26),
    SD_CMD_RESERVED(27),
    SD_CMD_RESERVED(28),
    SD_CMD_RESERVED(29),
    SD_CMD_RESERVED(30),
    SD_CMD_RESERVED(31),
    SD_CMD_RESERVED(32),
    SD_CMD_RESERVED(33),
    SD_CMD_RESERVED(34),
    SD_CMD_RESERVED(35),
    SD_CMD_RESERVED(36),
    SD_CMD_RESERVED(37),
    SD_CMD_RESERVED(38),
    SD_CMD_RESERVED(39),
    SD_CMD_RESERVED(40),
    SD_CMD_INDEX(41) | SD_RESP_R3,
    SD_CMD_INDEX(42) | SD_RESP_R1,
    SD_CMD_RESERVED(43),
    SD_CMD_RESERVED(44),
    SD_CMD_RESERVED(45),
    SD_CMD_RESERVED(46),
    SD_CMD_RESERVED(47),
    SD_CMD_RESERVED(48),
    SD_CMD_RESERVED(49),
    SD_CMD_RESERVED(50),
    SD_CMD_INDEX(51) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_RESERVED(52),
    SD_CMD_RESERVED(53),
    SD_CMD_RESERVED(54),
    SD_CMD_RESERVED(55),
    SD_CMD_RESERVED(56),
    SD_CMD_RESERVED(57),
    SD_CMD_RESERVED(58),
    SD_CMD_RESERVED(59),
    SD_CMD_RESERVED(60),
    SD_CMD_RESERVED(61),
    SD_CMD_RESERVED(62),
    SD_CMD_RESERVED(63)
};

// The actual command indices
#define GO_IDLE_STATE           0
#define ALL_SEND_CID            2
#define SEND_RELATIVE_ADDR      3
#define SET_DSR                 4
#define IO_SET_OP_COND          5
#define SWITCH_FUNC             6
#define SELECT_CARD             7
#define DESELECT_CARD           7
#define SELECT_DESELECT_CARD    7
#define SEND_IF_COND            8
#define SEND_CSD                9
#define SEND_CID                10
#define VOLTAGE_SWITCH          11
#define STOP_TRANSMISSION       12
#define SEND_STATUS             13
#define GO_INACTIVE_STATE       15
#define SET_BLOCKLEN            16
#define READ_SINGLE_BLOCK       17
#define READ_MULTIPLE_BLOCK     18
#define SEND_TUNING_BLOCK       19
#define SPEED_CLASS_CONTROL     20
#define SET_BLOCK_COUNT         23
#define WRITE_BLOCK             24
#define WRITE_MULTIPLE_BLOCK    25
#define PROGRAM_CSD             27
#define SET_WRITE_PROT          28
#define CLR_WRITE_PROT          29
#define SEND_WRITE_PROT         30
#define ERASE_WR_BLK_START      32
#define ERASE_WR_BLK_END        33
#define ERASE                   38
#define LOCK_UNLOCK             42
#define APP_CMD                 55
#define GEN_CMD                 56

#define IS_APP_CMD              0x80000000
#define ACMD(a)                 (a | IS_APP_CMD)
#define SET_BUS_WIDTH           (6 | IS_APP_CMD)
#define SD_STATUS               (13 | IS_APP_CMD)
#define SEND_NUM_WR_BLOCKS      (22 | IS_APP_CMD)
#define SET_WR_BLK_ERASE_COUNT  (23 | IS_APP_CMD)
#define SD_SEND_OP_COND         (41 | IS_APP_CMD)
#define SET_CLR_CARD_DETECT     (42 | IS_APP_CMD)
#define SEND_SCR                (51 | IS_APP_CMD)

#define SD_RESET_CMD            (1 << 25)
#define SD_RESET_DAT            (1 << 26)
#define SD_RESET_ALL            (1 << 24)

#define SD_GET_CLOCK_DIVIDER_FAIL	0xffffffff

#define TIMEOUT_WAIT(stop_if_true, usec)        \
do {                            \
    uint32_t touttimer = 0; \
    do {                     \
        if ((stop_if_true) | (touttimer == 100)) {  \
            break;  \
        } else {    \
            udelay(usec/100);   \
            touttimer += 1;  \
        }   \
    } while(1);            \
} while(0);

void set_gpio_func(uint32_t pin, uint32_t mode)
{
    uint32_t nSelReg = GPIO_GPFSEL0 + (pin / 10) * 4;
    uint32_t nShift = (pin % 10) * 3;
    uint32_t nValue = mmio_read(nSelReg);
    nValue &= ~(7 << nShift);
    nValue |= mode << nShift;
    mmio_write(nSelReg, nValue);
}

// Support for unaligned data access
static inline void write_word(uint32_t val, uint8_t *buf, int offset)
{
    buf[offset + 0] = val & 0xff;
    buf[offset + 1] = (val >> 8) & 0xff;
    buf[offset + 2] = (val >> 16) & 0xff;
    buf[offset + 3] = (val >> 24) & 0xff;
}

static inline uint32_t read_word(uint8_t *buf, int offset)
{
    uint32_t b0 = buf[offset + 0] & 0xff;
    uint32_t b1 = buf[offset + 1] & 0xff;
    uint32_t b2 = buf[offset + 2] & 0xff;
    uint32_t b3 = buf[offset + 3] & 0xff;

    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

// Support for BE to LE conversion
#ifdef __GNUC__
#define byte_swap __builtin_bswap32
#else
static inline uint32_t byte_swap(uint32_t in)
{
    uint32_t b0 = in & 0xff;
    uint32_t b1 = (in >> 8) & 0xff;
    uint32_t b2 = (in >> 16) & 0xff;
    uint32_t b3 = (in >> 24) & 0xff;
    uint32_t val_ret = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
    return val_ret;
}
#endif // __GNUC__

/* Power off the SD card */
static void sd_power_off()
{
	uint32_t control0 = mmio_read(EMMC_BASE + EMMC_CONTROL0);
	control0 &= ~(1 << 8);	// Set SD Bus Power bit off in Power Control Register
	mmio_write(EMMC_BASE + EMMC_CONTROL0, control0);
}

static int bcm_2708_power_cycle()
{
	if(bcm_2708_power_off() < 0)
		return -1;

	udelay(5000);

	return bcm_2708_power_on();
}

// Set the clock dividers to generate a target value
static uint32_t sd_get_clock_divider(uint32_t base_clock, uint32_t target_rate)
{
    // TODO: implement use of preset value registers

    uint32_t targetted_divisor = 0;
    if(target_rate > base_clock)
        targetted_divisor = 1;
    else
    {
        targetted_divisor = base_clock / target_rate;
        uint32_t mod = base_clock % target_rate;
        if(mod)
            targetted_divisor--;
    }

    // Decide on the clock mode to use

    // Currently only 10-bit divided clock mode is supported

    if(hci_ver >= 2)
    {
        // HCI version 3 or greater supports 10-bit divided clock mode
        // This requires a power-of-two divider

        // Find the first bit set
        int divisor = -1;
        for(int first_bit = 31; first_bit >= 0; first_bit--)
        {
            uint32_t bit_test = (1 << first_bit);
            if(targetted_divisor & bit_test)
            {
                divisor = first_bit;
                targetted_divisor &= ~bit_test;
                if(targetted_divisor)
                {
                    // The divisor is not a power-of-two, increase it
                    divisor++;
                }
                break;
            }
        }

        if(divisor == -1)
            divisor = 31;
        if(divisor >= 32)
            divisor = 31;

        if(divisor != 0)
            divisor = (1 << (divisor - 1));

        if(divisor >= 0x400)
            divisor = 0x3ff;

        uint32_t freq_select = divisor & 0xff;
        uint32_t upper_bits = (divisor >> 8) & 0x3;
        uint32_t val_ret = (freq_select << 8) | (upper_bits << 6) | (0 << 5);

#ifdef EMMC_DEBUG
        int denominator = 1;
        if(divisor != 0)
            denominator = divisor * 2;
        int actual_clock = base_clock / denominator;
        printk("EMMC: base_clock: %d, target_rate: %d, divisor: %x, "
               "actual_clock: %d, val_ret: %x\n", base_clock, target_rate,
               divisor, actual_clock, val_ret);
#endif

        return val_ret;
    }
    else
    {
        printk("EMMC: unsupported host version\n");
        return SD_GET_CLOCK_DIVIDER_FAIL;
    }

}

// Switch the clock rate whilst running
static int sd_switch_clock_rate(uint32_t base_clock, uint32_t target_rate)
{
    // Decide on an appropriate divider
    uint32_t divider = sd_get_clock_divider(base_clock, target_rate);
    if(divider == SD_GET_CLOCK_DIVIDER_FAIL)
    {
        printk("EMMC: couldn't get a valid divider for target rate %d Hz\n",
               target_rate);
        return -1;
    }

    // Wait for the command inhibit (CMD and DAT) bits to clear
    while(mmio_read(EMMC_BASE + EMMC_STATUS) & 0x3)
        udelay(1000);

    // Set the SD clock off
    uint32_t control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
    control1 &= ~(1 << 2);
    mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
    udelay(2000);

    // Write the new divider
	control1 &= ~0xffe0;		// Clear old setting + clock generator select
    control1 |= divider;
    mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
    udelay(2000);

    // Enable the SD clock
    control1 |= (1 << 2);
    mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
    udelay(2000);

#ifdef EMMC_DEBUG
    printk("EMMC: successfully set clock rate to %d Hz\n", target_rate);
#endif
    return 0;
}

// Reset the CMD line
static int sd_reset_cmd()
{
    uint32_t control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
	control1 |= SD_RESET_CMD;
	mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
	TIMEOUT_WAIT((mmio_read(EMMC_BASE + EMMC_CONTROL1) & SD_RESET_CMD) == 0, 1000000);
	if((mmio_read(EMMC_BASE + EMMC_CONTROL1) & SD_RESET_CMD) != 0)
	{
		printk("EMMC: CMD line did not reset properly\n");
		return -1;
	}
	return 0;
}

// Reset the CMD line
static int sd_reset_dat()
{
    uint32_t control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
	control1 |= SD_RESET_DAT;
	mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
	TIMEOUT_WAIT((mmio_read(EMMC_BASE + EMMC_CONTROL1) & SD_RESET_DAT) == 0, 1000000);
	if((mmio_read(EMMC_BASE + EMMC_CONTROL1) & SD_RESET_DAT) != 0)
	{
		printk("EMMC: DAT line did not reset properly\n");
		return -1;
	}
	return 0;
}

static void sd_issue_command_int(struct emmc_block_dev *dev, uint32_t cmd_reg, uint32_t argument, uint32_t timeout)
{
    dev->last_cmd_reg = cmd_reg;
    dev->last_cmd_success = 0;
    uint32_t chk = 0;

    // This is as per HCSS 3.7.1.1/3.7.2.2

    // Check Command Inhibit
    while((chk = mmio_read(EMMC_BASE + EMMC_STATUS)) & 0x1)
        udelay(1000);
    printk("INHBIT: %x\n", chk);

    // Is the command with busy?
    if((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B)
    {
        // With busy

        // Is is an abort command?
        if((cmd_reg & SD_CMD_TYPE_MASK) != SD_CMD_TYPE_ABORT)
        {
            // Not an abort command

            // Wait for the data line to be free
            while((chk = mmio_read(EMMC_BASE + EMMC_STATUS)) & 0x2)
                udelay(1000);
            printk("STATUS: %d\n", chk);
        }
    }

    // Is this a DMA transfer?
    int is_sdma = 0;
    if((cmd_reg & SD_CMD_ISDATA) && (dev->use_sdma))
    {
#ifdef EMMC_DEBUG
        printk("SD: performing SDMA transfer, current INTERRUPT: %x\n",
               mmio_read(EMMC_BASE + EMMC_INTERRUPT));
#endif
        is_sdma = 1;
    }

    if(is_sdma)
    {
        // Set system address register (ARGUMENT2 in RPi)

        // We need to define a 4 kiB aligned buffer to use here
        // Then convert its virtual address to a bus address
        mmio_write(EMMC_BASE + EMMC_ARG2, SDMA_BUFFER_PA);
    }

    // Set block size and block count
    // For now, block size = 512 bytes, block count = 1,
    //  host SDMA buffer boundary = 4 kiB
    if(dev->blocks_to_transfer > 0xffff)
    {
        printk("SD: blocks_to_transfer too great (%d)\n",
               dev->blocks_to_transfer);
        dev->last_cmd_success = 0;
        return;
    }
    uint32_t blksizecnt = dev->block_size | (dev->blocks_to_transfer << 16);
    mmio_write(EMMC_BASE + EMMC_BLKSIZECNT, blksizecnt);

    // Set argument 1 reg
    mmio_write(EMMC_BASE + EMMC_ARG1, argument);

    if(is_sdma)
    {
        // Set Transfer mode register
        cmd_reg |= SD_CMD_DMA;
    }

    // Set command reg
    mmio_write(EMMC_BASE + EMMC_CMDTM, cmd_reg);

    udelay(2000);

    // Wait for command complete interrupt
    TIMEOUT_WAIT(mmio_read(EMMC_BASE + EMMC_INTERRUPT) & 0x8001, timeout);
    uint32_t irpts = mmio_read(EMMC_BASE + EMMC_INTERRUPT);
    // Clear command complete status
    mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff0001);
    printk("IRPTS: %x\n", irpts);

    // Test for errors
    if((irpts & 0xfff00001) != 0x1)
    {
#ifdef EMMC_DEBUG
        printk("SD: error occured whilst waiting for command complete interrupt\n");
#endif
        dev->last_error = irpts & 0xfff00000;
        dev->last_interrupt = irpts;
        return;
    }

    udelay(2000);

    // Get response data
    switch(cmd_reg & SD_CMD_RSPNS_TYPE_MASK)
    {
        case SD_CMD_RSPNS_TYPE_48:
        case SD_CMD_RSPNS_TYPE_48B:
            dev->last_r0 = mmio_read(EMMC_BASE + EMMC_RESP0);
            break;

        case SD_CMD_RSPNS_TYPE_136:
            dev->last_r0 = mmio_read(EMMC_BASE + EMMC_RESP0);
            dev->last_r1 = mmio_read(EMMC_BASE + EMMC_RESP1);
            dev->last_r2 = mmio_read(EMMC_BASE + EMMC_RESP2);
            dev->last_r3 = mmio_read(EMMC_BASE + EMMC_RESP3);
            printk("RESPONSE: %x%x%x%x\n", dev->last_r0, dev->last_r1, dev->last_r2, dev->last_r3);
            break;
    }

    // If with data, wait for the appropriate interrupt
    if((cmd_reg & SD_CMD_ISDATA) && (is_sdma == 0))
    {
        uint32_t wr_irpt;
        int is_write = 0;
        if(cmd_reg & SD_CMD_DAT_DIR_CH)
            wr_irpt = (1 << 5);     // read
        else
        {
            is_write = 1;
            wr_irpt = (1 << 4);     // write
        }

        int cur_block = 0;
        uint32_t *cur_buf_addr = (uint32_t *)dev->buf;
        while(cur_block < dev->blocks_to_transfer)
        {
#ifdef EMMC_DEBUG
			if(dev->blocks_to_transfer > 1)
				printk("SD: multi block transfer, awaiting block %d ready\n",
				cur_block);
#endif
            TIMEOUT_WAIT(mmio_read(EMMC_BASE + EMMC_INTERRUPT) & (wr_irpt | 0x8000), timeout);
            irpts = mmio_read(EMMC_BASE + EMMC_INTERRUPT);
            mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff0000 | wr_irpt);
            printk("IRPTS: %x\n", irpts);
            if((irpts & (0xffff0000 | wr_irpt)) != wr_irpt)
            {
#ifdef EMMC_DEBUG
            printk("SD: error occured whilst waiting for data ready interrupt\n");
#endif
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            // Transfer the block
            size_t cur_byte_no = 0;
            while(cur_byte_no < dev->block_size)
            {
                if(is_write)
				{
					uint32_t data = read_word((uint8_t *)cur_buf_addr, 0);
                    mmio_write(EMMC_BASE + EMMC_DATA, data);
				}
                else
				{
					uint32_t data = mmio_read(EMMC_BASE + EMMC_DATA);
					write_word(data, (uint8_t *)cur_buf_addr, 0);
				}
                cur_byte_no += 4;
                cur_buf_addr++;
            }

#ifdef EMMC_DEBUG
			printk("SD: block %d transfer complete\n", cur_block);
#endif

            cur_block++;
        }
    }

    // Wait for transfer complete (set if read/write transfer or with busy)
    if((((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) ||
       (cmd_reg & SD_CMD_ISDATA)) && (is_sdma == 0))
    {
        // First check command inhibit (DAT) is not already 0
        if((mmio_read(EMMC_BASE + EMMC_STATUS) & 0x2) == 0)
            mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff0002);
        else
        {
            TIMEOUT_WAIT(mmio_read(EMMC_BASE + EMMC_INTERRUPT) & 0x8002, timeout);
            irpts = mmio_read(EMMC_BASE + EMMC_INTERRUPT);
            mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff0002);

            // Handle the case where both data timeout and transfer complete
            //  are set - transfer complete overrides data timeout: HCSS 2.2.17
            if(((irpts & 0xffff0002) != 0x2) && ((irpts & 0xffff0002) != 0x100002))
            {
#ifdef EMMC_DEBUG
                printk("SD: error occured whilst waiting for transfer complete interrupt\n");
#endif
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }
            mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff0002);
        }
    }
    else if (is_sdma)
    {
        // For SDMA transfers, we have to wait for either transfer complete,
        //  DMA int or an error

        // First check command inhibit (DAT) is not already 0
        if((mmio_read(EMMC_BASE + EMMC_STATUS) & 0x2) == 0)
            mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff000a);
        else
        {
            TIMEOUT_WAIT(mmio_read(EMMC_BASE + EMMC_INTERRUPT) & 0x800a, timeout);
            irpts = mmio_read(EMMC_BASE + EMMC_INTERRUPT);
            mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff000a);

            // Detect errors
            if((irpts & 0x8000) && ((irpts & 0x2) != 0x2))
            {
#ifdef EMMC_DEBUG
                printk("SD: error occured whilst waiting for transfer complete interrupt\n");
#endif
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            // Detect DMA interrupt without transfer complete
            // Currently not supported - all block sizes should fit in the
            //  buffer
            if((irpts & 0x8) && ((irpts & 0x2) != 0x2))
            {
#ifdef EMMC_DEBUG
                printk("SD: error: DMA interrupt occured without transfer complete\n");
#endif
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            // Detect transfer complete
            if(irpts & 0x2)
            {
#ifdef EMMC_DEBUG
                printk("SD: SDMA transfer complete");
#endif
                // Transfer the data to the user buffer
                memcpy(dev->buf, (const void *)SDMA_BUFFER, dev->block_size);
            }
            else
            {
                // Unknown error
#ifdef EMMC_DEBUG
                if(irpts == 0)
                    printk("SD: timeout waiting for SDMA transfer to complete\n");
                else
                    printk("SD: unknown SDMA transfer error\n");

                printk("SD: INTERRUPT: %x, STATUS %x\n", irpts,
                       mmio_read(EMMC_BASE + EMMC_STATUS));
#endif

                if((irpts == 0) && ((mmio_read(EMMC_BASE + EMMC_STATUS) & 0x3) == 0x2))
                {
                    // The data transfer is ongoing, we should attempt to stop
                    //  it
#ifdef EMMC_DEBUG
                    printk("SD: aborting transfer\n");
#endif
                    mmio_write(EMMC_BASE + EMMC_CMDTM, sd_commands[STOP_TRANSMISSION]);

#ifdef EMMC_DEBUG
                    // pause to let us read the screen
                    udelay(2000000);
#endif
                }
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }
        }
    }

    // Return success
    dev->last_cmd_success = 1;
}

static void sd_handle_card_interrupt(struct emmc_block_dev *dev)
{
    // Handle a card interrupt

#ifdef EMMC_DEBUG
    uint32_t status = mmio_read(EMMC_BASE + EMMC_STATUS);

    printk("SD: card interrupt\n");
    printk("SD: controller status: %x\n", status);
#endif

    // Get the card status
    if(dev->card_rca)
    {
        sd_issue_command_int(dev, sd_commands[SEND_STATUS], dev->card_rca << 16,
                         500000);
        if(FAIL(dev))
        {
#ifdef EMMC_DEBUG
            printk("SD: unable to get card status\n");
#endif
        }
        else
        {
#ifdef EMMC_DEBUG
            printk("SD: card status: %x\n", dev->last_r0);
#endif
        }
    }
    else
    {
#ifdef EMMC_DEBUG
        printk("SD: no card currently selected\n");
#endif
    }
}

static void sd_handle_interrupts(struct emmc_block_dev *dev)
{
    uint32_t irpts = mmio_read(EMMC_BASE + EMMC_INTERRUPT);
    uint32_t reset_mask = 0;

    if(irpts & SD_COMMAND_COMPLETE)
    {
#ifdef EMMC_DEBUG
        printk("SD: spurious command complete interrupt\n");
#endif
        reset_mask |= SD_COMMAND_COMPLETE;
    }

    if(irpts & SD_TRANSFER_COMPLETE)
    {
#ifdef EMMC_DEBUG
        printk("SD: spurious transfer complete interrupt\n");
#endif
        reset_mask |= SD_TRANSFER_COMPLETE;
    }

    if(irpts & SD_BLOCK_GAP_EVENT)
    {
#ifdef EMMC_DEBUG
        printk("SD: spurious block gap event interrupt\n");
#endif
        reset_mask |= SD_BLOCK_GAP_EVENT;
    }

    if(irpts & SD_DMA_INTERRUPT)
    {
#ifdef EMMC_DEBUG
        printk("SD: spurious DMA interrupt\n");
#endif
        reset_mask |= SD_DMA_INTERRUPT;
    }

    if(irpts & SD_BUFFER_WRITE_READY)
    {
#ifdef EMMC_DEBUG
        printk("SD: spurious buffer write ready interrupt\n");
#endif
        reset_mask |= SD_BUFFER_WRITE_READY;
        sd_reset_dat();
    }

    if(irpts & SD_BUFFER_READ_READY)
    {
#ifdef EMMC_DEBUG
        printk("SD: spurious buffer read ready interrupt\n");
#endif
        reset_mask |= SD_BUFFER_READ_READY;
        sd_reset_dat();
    }

    if(irpts & SD_CARD_INSERTION)
    {
#ifdef EMMC_DEBUG
        printk("SD: card insertion detected\n");
#endif
        reset_mask |= SD_CARD_INSERTION;
    }

    if(irpts & SD_CARD_REMOVAL)
    {
#ifdef EMMC_DEBUG
        printk("SD: card removal detected\n");
#endif
        reset_mask |= SD_CARD_REMOVAL;
        dev->card_removal = 1;
    }

    if(irpts & SD_CARD_INTERRUPT)
    {
#ifdef EMMC_DEBUG
        printk("SD: card interrupt detected\n");
#endif
        sd_handle_card_interrupt(dev);
        reset_mask |= SD_CARD_INTERRUPT;
    }

    if(irpts & 0x8000)
    {
#ifdef EMMC_DEBUG
        printk("SD: spurious error interrupt: %x\n", irpts);
#endif
        reset_mask |= 0xffff0000;
    }

    mmio_write(EMMC_BASE + EMMC_INTERRUPT, reset_mask);
}

void sd_command(struct emmc_block_dev *dev, uint32_t command, uint32_t arg, uint32_t timeout)
{

#ifdef EMMC_DEBUG
    printk("SD: issuing command CMD%d\n", command);
#endif

    if(sd_commands[command] == SD_CMD_RESERVED(0))
    {
        printk("SD: invalid command CMD%d\n", command);
        dev->last_cmd_success = 0;
        return;
    }

    dev->last_cmd = command;
    sd_issue_command_int(dev, sd_commands[command], arg, timeout);

#ifdef EMMC_DEBUG
    if(FAIL(dev)) {
        printk("SD: error issuing command: interrupts %x: ", dev->last_interrupt);
        if(dev->last_error == 0) {
            printk("TIMEOUT\n");
        }
    } else {
        printk("SD: command completed successfully\n");
    }
#endif
    return;
}

void sd_issue_command(struct emmc_block_dev *dev, uint32_t command, uint32_t argument, uint32_t timeout)
{
    // First, handle any pending interrupts
    sd_handle_interrupts(dev);

    // Stop the command issue if it was the card remove interrupt that was
    //  handled
    if(dev->card_removal)
    {
        dev->last_cmd_success = 0;
        return;
    }

    // Now run the appropriate commands by calling sd_issue_command_int()
    if(command & IS_APP_CMD)
    {
        command &= 0xff;
#ifdef EMMC_DEBUG
        printk("SD: issuing command ACMD%d => %x\n", command, sd_acommands[command]);
#endif

        if(sd_acommands[command] == SD_CMD_RESERVED(0))
        {
            printk("SD: invalid command ACMD%d\n", command);
            dev->last_cmd_success = 0;
            return;
        }
        dev->last_cmd = APP_CMD;

        uint32_t rca = 0;
        if(dev->card_rca)
            rca = dev->card_rca << 16;
        sd_issue_command_int(dev, sd_commands[APP_CMD], rca, timeout);
        if(dev->last_cmd_success)
        {
            dev->last_cmd = command | IS_APP_CMD;
            sd_issue_command_int(dev, sd_acommands[command], argument, timeout);
        }
    }
    else
    {
#ifdef EMMC_DEBUG
        printk("SD: issuing command CMD%d => %x\n", command, sd_commands[command]);
#endif

        if(sd_commands[command] == SD_CMD_RESERVED(0))
        {
            printk("SD: invalid command CMD%d\n", command);
            dev->last_cmd_success = 0;
            return;
        }

        dev->last_cmd = command;
        sd_issue_command_int(dev, sd_commands[command], argument, timeout);
    }

#ifdef EMMC_DEBUG
    if(FAIL(dev)) {
        printk("SD: error issuing command: interrupts %x: ", dev->last_interrupt);
        if(dev->last_error == 0) {
            printk("TIMEOUT\n");
        }
    } else {
        printk("SD: command completed successfully\n");
    }
#endif
    return;
}

void sd_gpio(void) {
    uint32_t r;
    // GPIO_CLK, GPIO_CMD
    r = mmio_read(GPIO_GPFSEL3);
    r |= (7<<(4*3)) | (7<<(5*3));
    mmio_write(GPIO_GPFSEL3, r);
    mmio_write(GPIO_GPPUD, 2);
    udelay(150);
    mmio_write(GPIO_GPPUDCLK1, (1<<3));
    udelay(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = mmio_read(GPIO_GPFSEL3);
    r |= (7<<(6*3)) | (7<<(7*3)) | (7<<(8*3)) | (7<<(9*3));
    mmio_write(GPIO_GPFSEL3, r);
    mmio_write(GPIO_GPPUD, 2);
    udelay(150);
    mmio_write(GPIO_GPPUDCLK1, (1<<4) | (1<<5) | (1<<6) | (1<<7));
    udelay(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);
}

int sd_init(struct block_device *dev)
{
    // Check the sanity of the sd_commands and sd_acommands structures
    if(sizeof(sd_commands) != (64 * sizeof(uint32_t)))
    {
        printk("EMMC: fatal error, sd_commands of incorrect size: %d"
               " expected %d\n", sizeof(sd_commands),
               64 * sizeof(uint32_t));
        return -1;
    }
    if(sizeof(sd_acommands) != (64 * sizeof(uint32_t)))
    {
        printk("EMMC: fatal error, sd_acommands of incorrect size: %d"
               " expected %d\n", sizeof(sd_acommands),
               64 * sizeof(uint32_t));
        return -1;
    }

    sd_gpio();

    uint32_t d = mmio_read(0xfe2000d0);
    printk("Default mode:       %d\n", d);
    d &= 0xfffffffc;
    d |= 0x2;
    mmio_write(0xfe2000d0, d);
    d = mmio_read(0xfe2000d0);
    printk("Changed to mode:    %d\n", d);

#if SDHCI_IMPLEMENTATION == SDHCI_IMPLEMENTATION_BCM_2708
	// Power cycle the card to ensure its in its startup state
	if(bcm_2708_power_cycle() != 0)
	{
		printk("EMMC: BCM2708 controller did not power cycle successfully\n");
		return -1;
	}
#ifdef EMMC_DEBUG
	printk("EMMC: BCM2708 controller power-cycled\n");
#endif
#endif

    for (uint32_t i = 0; i < 6; i++) {
        set_gpio_func(34 + i, 0x0);
        set_gpio_func(48 + i, 0x7);
    }

    // Reset supply to 3.3v
    mmio_write(GPIO_GPCLR0, 0b10000);
    printk("SET SUPPLY TO 3.3V\n");

	// Read the controller version
	uint32_t ver = mmio_read(EMMC_BASE + EMMC_SLOTISR_VER);
	uint32_t vendor = ver >> 24;
	uint32_t sdversion = (ver >> 16) & 0xff;
	uint32_t slot_status = ver & 0xff;
	printk("EMMC: vendor %x, sdversion %x, slot_status %x\n", vendor, sdversion, slot_status);
	hci_ver = sdversion;

	if(hci_ver < 2)
	{
		printk("EMMC: only SDHCI versions >= 3.0 are supported\n");
		return -1;
	}

	// Reset the controller
#ifdef EMMC_DEBUG
	printk("EMMC: resetting controller\n");
#endif
	uint32_t control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
	control1 |= (1 << 24);
	// Disable clock
	control1 &= ~(1 << 2);
	control1 &= ~(1 << 0);
	mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
	TIMEOUT_WAIT((mmio_read(EMMC_BASE + EMMC_CONTROL1) & (0x7 << 24)) == 0, 1000000);
	if((mmio_read(EMMC_BASE + EMMC_CONTROL1) & (0x7 << 24)) != 0)
	{
		printk("EMMC: controller did not reset properly\n");
		return -1;
	}
#ifdef EMMC_DEBUG
	printk("EMMC: control0: %x, control1: %x, control2: %x\n",
			mmio_read(EMMC_BASE + EMMC_CONTROL0),
			mmio_read(EMMC_BASE + EMMC_CONTROL1),
            mmio_read(EMMC_BASE + EMMC_CONTROL2));
#endif

    sd_disable_low_power();
    // Enable SD Bus Power VDD1 at 3.3V
	uint32_t control0 = mmio_read(EMMC_BASE + EMMC_CONTROL0);
	control0 |= 0x0F << 8;
	mmio_write(EMMC_BASE + EMMC_CONTROL0, control0);
	udelay(2000);

	// Read the capabilities registers
	capabilities_0 = mmio_read(EMMC_BASE + EMMC_CAPABILITIES_0);
	capabilities_1 = mmio_read(EMMC_BASE + EMMC_CAPABILITIES_1);
#ifdef EMMC_DEBUG
	printk("EMMC: capabilities: %x%x\n", capabilities_1, capabilities_0);
#endif

	// Check for a valid card
#ifdef EMMC_DEBUG
	printk("EMMC: checking for an inserted card\n");
#endif
    TIMEOUT_WAIT(mmio_read(EMMC_BASE + EMMC_STATUS) & (1 << 16), 500000);
	uint32_t status_reg = mmio_read(EMMC_BASE + EMMC_STATUS);
	if((status_reg & (1 << 16)) == 0)
	{
		printk("EMMC: no card inserted\n");
		return -1;
	}
#ifdef EMMC_DEBUG
	printk("EMMC: status: %x\n", status_reg);
#endif

	// Clear control2
	mmio_write(EMMC_BASE + EMMC_CONTROL2, 0);

	// Get the base clock rate
	uint32_t base_clock = sd_get_base_clock_hz();
	if(base_clock == 0)
	{
	    printk("EMMC: assuming clock rate to be 100MHz\n");
	    base_clock = 100000000;
	}

	// Set clock rate to something slow
#ifdef EMMC_DEBUG
	printk("EMMC: setting clock rate\n");
#endif
	control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
	control1 |= 1;			// enable clock

	// Set to identification frequency (400 kHz)
	uint32_t f_id = sd_get_clock_divider(base_clock, SD_CLOCK_ID);
	if(f_id == SD_GET_CLOCK_DIVIDER_FAIL)
	{
		printk("EMMC: unable to get a valid clock divider for ID frequency\n");
		return -1;
	}
    control1 &= ~(0x3FF << 6);
	control1 |= f_id;

	control1 &= ~(0xF << 16);
    control1 |= (11 << 16);
	mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
	TIMEOUT_WAIT(mmio_read(EMMC_BASE + EMMC_CONTROL1) & 0x2, 0x1000000);
	if((mmio_read(EMMC_BASE + EMMC_CONTROL1) & 0x2) == 0)
	{
		printk("EMMC: controller's clock did not stabilise within 1 second\n");
		return -1;
	}
#ifdef EMMC_DEBUG
	printk("EMMC: control0: %x, control1: %x\n",
			mmio_read(EMMC_BASE + EMMC_CONTROL0),
			mmio_read(EMMC_BASE + EMMC_CONTROL1));
#endif

	// Enable the SD clock
#ifdef EMMC_DEBUG
	printk("EMMC: enabling SD clock\n");
#endif
	udelay(2000);
	control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
	control1 |= 4;
	mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);
	udelay(2000);
#ifdef EMMC_DEBUG
	printk("EMMC: SD clock enabled\n");
#endif

	// Mask off sending interrupts to the ARM
	mmio_write(EMMC_BASE + EMMC_IRPT_EN, 0);
	// Reset interrupts
	mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffffffff);
	// Have all interrupts sent to the INTERRUPT register
	uint32_t irpt_mask = 0xffffffff & (~SD_CARD_INTERRUPT);
#ifdef SD_CARD_INTERRUPTS
    irpt_mask |= SD_CARD_INTERRUPT;
#endif
	mmio_write(EMMC_BASE + EMMC_IRPT_MASK, irpt_mask);

#ifdef EMMC_DEBUG
	printk("EMMC: interrupts disabled\n");
#endif
	udelay(2000);

    // Prepare the device structure
	struct emmc_block_dev *val_ret = &emmc_dev;

	memset(val_ret, 0, sizeof(struct emmc_block_dev));
	val_ret->bd.driver_name = driver_name;
	val_ret->bd.device_name = device_name;
	val_ret->bd.block_size = 512;
    val_ret->bd.supports_multiple_block_read = 1;
    val_ret->bd.supports_multiple_block_write = 1;
	val_ret->base_clock = base_clock;

#ifdef EMMC_DEBUG
	printk("EMMC: device structure created\n");
#endif

	// Send CMD0 to the card (reset to idle state)
    sd_issue_command(val_ret, GO_IDLE_STATE, 0, 10000000);
    printk("Returned from CMD0\n");
	if(FAIL(val_ret))
	{
        printk("SD: no CMD0 response\n");
        return -1;
	}

    // Send CMD8 to the card
	// Voltage supplied = 0x1 = 2.7-3.6V (standard)
	// Check pattern = 10101010b (as per PLSS 4.3.13) = 0xAA
#ifdef EMMC_DEBUG
    printk("SD: note a timeout error on the following command (CMD8) is normal "
           "and expected if the SD card version is less than 2.0\n");
#endif
    sd_issue_command(val_ret, SEND_IF_COND, 0x1aa, 10000000);
    printk("Returned from CMD8\n");
	int v2_later = 0;
	if(TIMEOUT(val_ret))
        v2_later = 0;
    else if(CMD_TIMEOUT(val_ret))
    {
        if(sd_reset_cmd() == -1)
            return -1;
        mmio_write(EMMC_BASE + EMMC_INTERRUPT, SD_ERR_MASK_CMD_TIMEOUT);
        v2_later = 0;
    }
    else if(FAIL(val_ret))
    {
        printk("SD: failure sending CMD8 (%x)\n", val_ret->last_interrupt);
        return -1;
    }
    else
    {
        if((val_ret->last_r0 & 0xfff) != 0x1aa)
        {
            printk("SD: unusable card\n");
#ifdef EMMC_DEBUG
            printk("SD: CMD8 response %x\n", val_ret->last_r0);
#endif
            return -1;
        }
        else
            v2_later = 1;
    }

    // Here we are supposed to check the response to CMD5 (HCSS 3.6)
    // It only returns if the card is a SDIO card
#ifdef EMMC_DEBUG
    printk("SD: note that a timeout error on the following command (CMD5) is "
           "normal and expected if the card is not a SDIO card.\n");
#endif
    sd_issue_command(val_ret, IO_SET_OP_COND, 0, 10000000);
    if(!TIMEOUT(val_ret))
    {
        if(CMD_TIMEOUT(val_ret))
        {
            if(sd_reset_cmd() == -1)
                return -1;
            mmio_write(EMMC_BASE + EMMC_INTERRUPT, SD_ERR_MASK_CMD_TIMEOUT);
        }
        else
        {
            printk("SD: SDIO card detected - not currently supported\n");
#ifdef EMMC_DEBUG
            printk("SD: CMD5 returned %x\n", val_ret->last_r0);
#endif
            return -1;
        }
    }

    // Call an inquiry ACMD41 (voltage window = 0) to get the OCR
#ifdef EMMC_DEBUG
    printk("SD: sending inquiry ACMD41\n");
#endif
    sd_issue_command(val_ret, ACMD(41), 0, 10000000);
    if(FAIL(val_ret))
    {
        printk("SD: inquiry ACMD41 failed\n");
        return -1;
    }
#ifdef EMMC_DEBUG
    printk("SD: inquiry ACMD41 returned %x\n", val_ret->last_r0);
#endif

	// Call initialization ACMD41
	int card_is_busy = 1;
	while(card_is_busy)
	{
	    uint32_t v2_flags = 0;
	    if(v2_later)
	    {
	        // Set SDHC support
	        v2_flags |= (1 << 30);

	        // Set 1.8v support
#ifdef SD_1_8V_SUPPORT
	        if(!val_ret->failed_voltage_switch)
                v2_flags |= (1 << 24);
#endif

            // Enable SDXC maximum performance
#ifdef SDXC_MAXIMUM_PERFORMANCE
            v2_flags |= (1 << 28);
#endif
	    }

	    sd_issue_command(val_ret, ACMD(41), 0x00ff8000 | v2_flags, 10000000);
	    if(FAIL(val_ret))
	    {
	        printk("SD: error issuing ACMD41\n");
	        return -1;
	    }

	    if((val_ret->last_r0 >> 31) & 0x1)
	    {
	        // Initialization is complete
	        val_ret->card_ocr = (val_ret->last_r0 >> 8) & 0xffff;
	        val_ret->card_supports_sdhc = (val_ret->last_r0 >> 30) & 0x1;

#ifdef SD_1_8V_SUPPORT
	        if(!val_ret->failed_voltage_switch)
                val_ret->card_supports_18v = (val_ret->last_r0 >> 24) & 0x1;
#endif

	        card_is_busy = 0;
	    }
	    else
	    {
	        // Card is still busy
#ifdef EMMC_DEBUG
            printk("SD: card is busy, retrying\n");
#endif
            udelay(500000);
	    }
	}

#ifdef EMMC_DEBUG
	printk("SD: card identified: OCR: %x, 1.8v support: %d, SDHC support: %d\n",
			val_ret->card_ocr, val_ret->card_supports_18v, val_ret->card_supports_sdhc);
#endif

    // At this point, we know the card is definitely an SD card, so will definitely
	//  support SDR12 mode which runs at 25 MHz
    sd_switch_clock_rate(base_clock, SD_CLOCK_NORMAL);

	// A small wait before the voltage switch
	udelay(5000);

	// Switch to 1.8V mode if possible
	if(val_ret->card_supports_18v)
	{
#ifdef EMMC_DEBUG
        printk("SD: switching to 1.8V mode\n");
#endif
	    // As per HCSS 3.6.1

	    // Send VOLTAGE_SWITCH
	    sd_issue_command(val_ret, VOLTAGE_SWITCH, 0, 100000);
	    if(FAIL(val_ret))
	    {
#ifdef EMMC_DEBUG
            printk("SD: error issuing VOLTAGE_SWITCH\n");
#endif
	        val_ret->failed_voltage_switch = 1;
			sd_power_off();
	        return sd_init((struct block_device *)val_ret);
	    }

	    // Disable SD clock
	    control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
	    control1 &= ~(1 << 2);
	    mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);

	    // Check DAT[3:0]
	    status_reg = mmio_read(EMMC_BASE + EMMC_STATUS);
	    uint32_t dat30 = (status_reg >> 20) & 0xf;
	    if(dat30 != 0)
	    {
#ifdef EMMC_DEBUG
            printk("SD: DAT[3:0] did not settle to 0\n");
#endif
	        val_ret->failed_voltage_switch = 1;
			sd_power_off();
	        return sd_init((struct block_device *)val_ret);
	    }

	    // Set 1.8V signal enable to 1
	    uint32_t control0 = mmio_read(EMMC_BASE + EMMC_CONTROL0);
	    control0 |= (1 << 8);
	    mmio_write(EMMC_BASE + EMMC_CONTROL0, control0);

	    // Wait 5 ms
	    udelay(5000);

	    // Check the 1.8V signal enable is set
	    control0 = mmio_read(EMMC_BASE + EMMC_CONTROL0);
	    if(((control0 >> 8) & 0x1) == 0)
	    {
#ifdef EMMC_DEBUG
            printk("SD: controller did not keep 1.8V signal enable high\n");
#endif
	        val_ret->failed_voltage_switch = 1;
			sd_power_off();
	        return sd_init((struct block_device *)&val_ret);
	    }

	    // Re-enable the SD clock
	    control1 = mmio_read(EMMC_BASE + EMMC_CONTROL1);
	    control1 |= (1 << 2);
	    mmio_write(EMMC_BASE + EMMC_CONTROL1, control1);

	    // Wait 1 ms
	    udelay(10000);

	    // Check DAT[3:0]
	    status_reg = mmio_read(EMMC_BASE + EMMC_STATUS);
	    dat30 = (status_reg >> 20) & 0xf;
	    if(dat30 != 0xf)
	    {
#ifdef EMMC_DEBUG
            printk("SD: DAT[3:0] did not settle to 1111b (%01x)\n", dat30);
#endif
	        val_ret->failed_voltage_switch = 1;
			sd_power_off();
	        return sd_init((struct block_device *)val_ret);
	    }

#ifdef EMMC_DEBUG
        printk("SD: voltage switch complete\n");
#endif
	}

	// Send CMD2 to get the cards CID
	sd_issue_command(val_ret, ALL_SEND_CID, 0, 100000);
	if(FAIL(val_ret))
	{
	    printk("SD: error sending ALL_SEND_CID\n");
	    return -1;
	}
	uint32_t card_cid_0 = val_ret->last_r0;
	uint32_t card_cid_1 = val_ret->last_r1;
	uint32_t card_cid_2 = val_ret->last_r2;
	uint32_t card_cid_3 = val_ret->last_r3;

#ifdef EMMC_DEBUG
	printk("SD: card CID: %x%x%x%x\n", card_cid_3, card_cid_2, card_cid_1, card_cid_0);
#endif
	uint32_t *dev_id = NULL;
    new(dev_id, 4, uint32_t);
	dev_id[0] = card_cid_0;
	dev_id[1] = card_cid_1;
	dev_id[2] = card_cid_2;
	dev_id[3] = card_cid_3;
	val_ret->bd.device_id = (uint8_t *)dev_id;
	val_ret->bd.dev_id_len = 4 * sizeof(uint32_t);

	// Send CMD3 to enter the data state
	sd_issue_command(val_ret, SEND_RELATIVE_ADDR, 0, 100000);
	if(FAIL(val_ret))
    {
        printk("SD: error sending SEND_RELATIVE_ADDR\n");
        del(val_ret);
        return -1;
    }

	uint32_t cmd3_resp = val_ret->last_r0;
#ifdef EMMC_DEBUG
	printk("SD: CMD3 response: %x\n", cmd3_resp);
#endif

	val_ret->card_rca = (cmd3_resp >> 16) & 0xffff;
	uint32_t crc_error = (cmd3_resp >> 15) & 0x1;
	uint32_t illegal_cmd = (cmd3_resp >> 14) & 0x1;
	uint32_t error = (cmd3_resp >> 13) & 0x1;
	uint32_t status = (cmd3_resp >> 9) & 0xf;
	uint32_t ready = (cmd3_resp >> 8) & 0x1;

	if(crc_error)
	{
		printk("SD: CRC error\n");
		del(val_ret);
		del(dev_id);
		return -1;
	}

	if(illegal_cmd)
	{
		printk("SD: illegal command\n");
		del(val_ret);
		del(dev_id);
		return -1;
	}

	if(error)
	{
		printk("SD: generic error\n");
		del(val_ret);
		del(dev_id);
		return -1;
	}

	if(!ready)
	{
		printk("SD: not ready for data\n");
		del(val_ret);
		del(dev_id);
		return -1;
	}

#ifdef EMMC_DEBUG
	printk("SD: RCA: %x\n", val_ret->card_rca);
#endif

	// Now select the card (toggles it to transfer state)
	sd_issue_command(val_ret, SELECT_CARD, val_ret->card_rca << 16, 100000);
	if(FAIL(val_ret))
	{
	    printk("SD: error sending CMD7\n");
	    del(val_ret);
	    return -1;
	}

	uint32_t cmd7_resp = val_ret->last_r0;
	status = (cmd7_resp >> 9) & 0xf;

	if((status != 3) && (status != 4))
	{
		printk("SD: invalid status (%d)\n", status);
		del(val_ret);
		del(dev_id);
		return -1;
	}

	// If not an SDHC card, ensure BLOCKLEN is 512 bytes
	if(!val_ret->card_supports_sdhc)
	{
	    sd_issue_command(val_ret, SET_BLOCKLEN, 512, 100000);
	    if(FAIL(val_ret))
	    {
	        printk("SD: error sending SET_BLOCKLEN\n");
	        del(val_ret);
	        return -1;
	    }
	}
	val_ret->block_size = 512;
	uint32_t controller_block_size = mmio_read(EMMC_BASE + EMMC_BLKSIZECNT);
	controller_block_size &= (~0xfff);
	controller_block_size |= 0x200;
	mmio_write(EMMC_BASE + EMMC_BLKSIZECNT, controller_block_size);

	// Get the cards SCR register
	val_ret->scr = NULL;
    new(val_ret->scr, 1, struct sd_scr);
	val_ret->buf = &val_ret->scr->scr[0];
	val_ret->block_size = 8;
	val_ret->blocks_to_transfer = 1;
	sd_issue_command(val_ret, SEND_SCR, 0, 100000);
	val_ret->block_size = 512;
	if(FAIL(val_ret))
	{
	    printk("SD: error sending SEND_SCR\n");
	    del(val_ret->scr);
        del(val_ret);
	    return -1;
	}

	// Determine card version
	// Note that the SCR is big-endian
	uint32_t scr0 = byte_swap(val_ret->scr->scr[0]);
	val_ret->scr->sd_version = SD_VER_UNKNOWN;
	uint32_t sd_spec = (scr0 >> (56 - 32)) & 0xf;
	uint32_t sd_spec3 = (scr0 >> (47 - 32)) & 0x1;
	uint32_t sd_spec4 = (scr0 >> (42 - 32)) & 0x1;
	val_ret->scr->sd_bus_widths = (scr0 >> (48 - 32)) & 0xf;
	if(sd_spec == 0)
        val_ret->scr->sd_version = SD_VER_1;
    else if(sd_spec == 1)
        val_ret->scr->sd_version = SD_VER_1_1;
    else if(sd_spec == 2)
    {
        if(sd_spec3 == 0)
            val_ret->scr->sd_version = SD_VER_2;
        else if(sd_spec3 == 1)
        {
            if(sd_spec4 == 0)
                val_ret->scr->sd_version = SD_VER_3;
            else if(sd_spec4 == 1)
                val_ret->scr->sd_version = SD_VER_4;
        }
    }

#ifdef EMMC_DEBUG
    printk("SD: &scr: %x\n", &val_ret->scr->scr[0]);
    printk("SD: SCR[0]: %x, SCR[1]: %x\n", val_ret->scr->scr[0], val_ret->scr->scr[1]);;
    printk("SD: SCR: %x%x\n", byte_swap(val_ret->scr->scr[0]), byte_swap(val_ret->scr->scr[1]));
    printk("SD: SCR: version %s, bus_widths %01x\n", sd_versions[val_ret->scr->sd_version],
           val_ret->scr->sd_bus_widths);
#endif

    if(val_ret->scr->sd_bus_widths & 0x4)
    {
        // Set 4-bit transfer mode (ACMD6)
        // See HCSS 3.4 for the algorithm
#ifdef SD_4BIT_DATA
#ifdef EMMC_DEBUG
        printk("SD: switching to 4-bit data mode\n");
#endif

        // Disable card interrupt in host
        uint32_t old_irpt_mask = mmio_read(EMMC_BASE + EMMC_IRPT_MASK);
        uint32_t new_iprt_mask = old_irpt_mask & ~(1 << 8);
        mmio_write(EMMC_BASE + EMMC_IRPT_MASK, new_iprt_mask);

        // Send ACMD6 to change the card's bit mode
        sd_issue_command(val_ret, SET_BUS_WIDTH, 0x2, 100000);
        if(FAIL(val_ret))
            printk("SD: switch to 4-bit data mode failed\n");
        else
        {
            // Change bit mode for Host
            uint32_t control0 = mmio_read(EMMC_BASE + EMMC_CONTROL0);
            control0 |= 0x2;
            mmio_write(EMMC_BASE + EMMC_CONTROL0, control0);

            // Re-enable card interrupt in host
            mmio_write(EMMC_BASE + EMMC_IRPT_MASK, old_irpt_mask);

#ifdef EMMC_DEBUG
                printk("SD: switch to 4-bit complete\n");
#endif
        }
#endif
    }

	printk("SD: found a valid version %s SD card\n", sd_versions[val_ret->scr->sd_version]);
#ifdef EMMC_DEBUG
	printk("SD: setup successful (status %d)\n", status);
#endif

	// Reset interrupt register
	mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffffffff);

	if(dev != NULL)
	    *dev = (*(struct block_device *)val_ret);

exit:
	return 0;
}

static int sd_ensure_data_mode(struct emmc_block_dev *edev)
{
	if(edev->card_rca == 0)
	{
		// Try again to initialise the card
		int ret = sd_init((struct block_device *)edev);
		if(ret != 0)
			return ret;
	}

#ifdef EMMC_DEBUG
	printk("SD: ensure_data_mode() obtaining status register for card_rca %x: ",
		edev->card_rca);
#endif

    sd_issue_command(edev, SEND_STATUS, edev->card_rca << 16, 500000);
    if(FAIL(edev))
    {
        printk("SD: ensure_data_mode() error sending CMD13\n");
        edev->card_rca = 0;
        return -1;
    }

	uint32_t status = edev->last_r0;
	uint32_t cur_state = (status >> 9) & 0xf;
#ifdef EMMC_DEBUG
	printk("status %d\n", cur_state);
#endif
	if(cur_state == 3)
	{
		// Currently in the stand-by state - select it
		sd_issue_command(edev, SELECT_CARD, edev->card_rca << 16, 500000);
		if(FAIL(edev))
		{
			printk("SD: ensure_data_mode() no response from CMD17\n");
			edev->card_rca = 0;
			return -1;
		}
	}
	else if(cur_state == 5)
	{
		// In the data transfer state - cancel the transmission
		sd_issue_command(edev, STOP_TRANSMISSION, 0, 500000);
		if(FAIL(edev))
		{
			printk("SD: ensure_data_mode() no response from CMD12\n");
			edev->card_rca = 0;
			return -1;
		}

		// Reset the data circuit
		sd_reset_dat();
	}
	else if(cur_state != 4)
	{
		// Not in the transfer state - re-initialise
		int ret = sd_init((struct block_device *)edev);
		if(ret != 0)
			return ret;
	}

	// Check again that we're now in the correct mode
	if(cur_state != 4)
	{
#ifdef EMMC_DEBUG
		printk("SD: ensure_data_mode() rechecking status: ");
#endif
        sd_issue_command(edev, SEND_STATUS, edev->card_rca << 16, 500000);
        if(FAIL(edev))
		{
			printk("SD: ensure_data_mode() no response from CMD13\n");
			edev->card_rca = 0;
			return -1;
		}
		status = edev->last_r0;
		cur_state = (status >> 9) & 0xf;

#ifdef EMMC_DEBUG
		printk("%d\n", cur_state);
#endif

		if(cur_state != 4)
		{
			printk("SD: unable to initialise SD card to "
					"data mode (state %d)\n", cur_state);
			edev->card_rca = 0;
			return -1;
		}
	}

	return 0;
}

#ifdef SDMA_SUPPORT
// We only support DMA transfers to buffers aligned on a 4 kiB boundary
static int sd_suitable_for_dma(void *buf)
{
    if((uintptr_t)buf & 0xfff)
        return 0;
    else
        return 1;
}
#endif

static int sd_do_data_command(struct emmc_block_dev *edev, int is_write, uint8_t *buf, size_t buf_size, uint32_t block_no)
{
	// PLSS table 4.20 - SDSC cards use byte addresses rather than block addresses
	if(!edev->card_supports_sdhc)
		block_no *= 512;

	// This is as per HCSS 3.7.2.1
	if(buf_size < edev->block_size)
	{
	    printk("SD: do_data_command() called with buffer size (%d) less than "
            "block size (%d)\n", buf_size, edev->block_size);
        return -1;
	}

	edev->blocks_to_transfer = buf_size / edev->block_size;
	if(buf_size % edev->block_size)
	{
	    printk("SD: do_data_command() called with buffer size (%d) not an "
            "exact multiple of block size (%d)\n", buf_size, edev->block_size);
        return -1;
	}
	edev->buf = buf;

	// Decide on the command to use
	int command;
	if(is_write)
	{
	    if(edev->blocks_to_transfer > 1)
            command = WRITE_MULTIPLE_BLOCK;
        else
            command = WRITE_BLOCK;
	}
	else
    {
        if(edev->blocks_to_transfer > 1)
            command = READ_MULTIPLE_BLOCK;
        else
            command = READ_SINGLE_BLOCK;
    }

	int retry_count = 0;
	int max_retries = 3;
	while(retry_count < max_retries)
	{
#ifdef SDMA_SUPPORT
	    // use SDMA for the first try only
	    if((retry_count == 0) && sd_suitable_for_dma(buf))
            edev->use_sdma = 1;
        else
        {
#ifdef EMMC_DEBUG
            printk("SD: retrying without SDMA\n");
#endif
            edev->use_sdma = 0;
        }
#else
        edev->use_sdma = 0;
#endif

        sd_issue_command(edev, command, block_no, 5000000);

        if(SUCCESS(edev))
            break;
        else
        {
            printk("SD: error sending CMD%d, ", command);
            printk("error = %x.  ", edev->last_error);
            retry_count++;
            if(retry_count < max_retries)
                printk("Retrying...\n");
            else
                printk("Giving up.\n");
        }
	}
	if(retry_count == max_retries)
    {
        edev->card_rca = 0;
        return -1;
    }

    return 0;
}

size_t sd_read(uint8_t *buf, size_t buf_size, uint32_t block_no)
{
	// Check the status of the card
	struct emmc_block_dev *edev = &emmc_dev;
    if(sd_ensure_data_mode(edev) != 0)
        return -1;

#ifdef EMMC_DEBUG
	printk("SD: read() card ready, reading from block %u\n", block_no);
#endif

    if(sd_do_data_command(edev, 0, buf, buf_size, block_no) < 0)
        return -1;

#ifdef EMMC_DEBUG
	printk("SD: data read successful\n");
#endif

	return buf_size;
}

size_t sd_write(uint8_t *buf, size_t buf_size, uint32_t block_no)
{
	// Check the status of the card
	struct emmc_block_dev *edev = &emmc_dev;
    if(sd_ensure_data_mode(edev) != 0)
        return -1;

#ifdef EMMC_DEBUG
	printk("SD: write() card ready, reading from block %u\n", block_no);
#endif

    if(sd_do_data_command(edev, 1, buf, buf_size, block_no) < 0)
        return -1;

#ifdef EMMC_DEBUG
	printk("SD: write read successful\n");
#endif

	return buf_size;
}

size_t sd_get_block_size()
{
    return emmc_dev.bd.block_size;
}

size_t sd_get_num_blocks()
{
    return emmc_dev.bd.num_blocks;
}