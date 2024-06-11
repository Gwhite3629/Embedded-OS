#ifndef _SD_H_
#define _SD_H_

#include "../stdlib.h"

#define CAST(X) ((X))

#define EMMC_OFFSET         (IO_BASE + 0x00300000)

// EMMC Offsets

#define EMMC_ARG2           CAST(EMMC_OFFSET + 0x00)
#define EMMC_BLKSIZECNT     CAST(EMMC_OFFSET + 0x04)
#define EMMC_ARG1           CAST(EMMC_OFFSET + 0x08)
#define EMMC_CMDTM          CAST(EMMC_OFFSET + 0x0C)
#define EMMC_RESP0          CAST(EMMC_OFFSET + 0x10)
#define EMMC_RESP1          CAST(EMMC_OFFSET + 0x14)
#define EMMC_RESP2          CAST(EMMC_OFFSET + 0x18)
#define EMMC_RESP3          CAST(EMMC_OFFSET + 0x1C)
#define EMMC_DATA           CAST(EMMC_OFFSET + 0x20)
#define EMMC_STATUS         CAST(EMMC_OFFSET + 0x24)
#define EMMC_CONTROL0       CAST(EMMC_OFFSET + 0x28)
#define EMMC_CONTROL1       CAST(EMMC_OFFSET + 0x2C)
#define EMMC_INTERRUPT      CAST(EMMC_OFFSET + 0x30)
#define EMMC_INT_MASK       CAST(EMMC_OFFSET + 0x34)
#define EMMC_INT_EN         CAST(EMMC_OFFSET + 0x38)
#define EMMC_CONTROL2       CAST(EMMC_OFFSET + 0x3C)
#define EMMC_SLOTISR_VER    CAST(EMMC_OFFSET + 0xFC)

// SD Command Flags
#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfffC9004
#define CMD_RCA_MASK        0xffff0000

// SD Commands
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x11220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_WRITE_SINGLE    0x18220000
#define CMD_WRITE_MULTI     0x19220022
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000 | CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000 | CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010 | CMD_NEED_APP)

// Status register controls
#define SR_READ_AVAILABLE   0x00000800
#define SR_WRITE_AVAILABLE  0x00000400
#define SR_DAT_INHIBIT      0x00000002
#define SR_CMD_INHIBIT      0x00000001
#define SR_APP_CMD          0x00000020

// Interrupt Register controls
#define INT_DATA_TIMEOUT    0x00100000
#define INT_CMD_TIMEOUT     0x00010000
#define INT_READ_RDY        0x00000020
#define INT_WRITE_RDY       0x00000010
#define INT_DATA_DONE       0x00000002
#define INT_CMD_DONE        0x00000001
#define INT_ERROR_MASK      0x017E8000

// Control Register controls
#define C0_SPI_MODE_EN      0x00100000
#define C0_HCTL_HS_EN       0x00000004
#define C0_HCTL_DWIDTH      0x00000002

#define C1_SRST_DATA        0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_DS        0x000F0000
#define C1_TOUNIT_MAX       0x000E0000
#define C1_CLK_GENSEL       0x00000020
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001

#define HOST_SPEC_NUM       0x00FF0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

#define SCR_SD_BUS_WIDTH    0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00FF8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51FF8000

#define CTRL_SYNC 0
#define GET_SECTOR_COUNT 1
#define GET_SECTOR_SIZE 2
#define GET_BLOCK_SIZE 3
#define CTRL_TRIM 4

#define STATUS_NOINIT   0x01
#define STATUS_NODISK   0x02
#define STATUS_PROTECT  0x04

err_t sd_init(void);
err_t sd_status(unsigned int mask);
err_t sd_read(char *buff, uint16_t sector, size_t n);
err_t sd_write(const char *buff, uint16_t sector, size_t n);
err_t sd_ioctl(unsigned int cmd, unsigned int arg);

#endif // _SD_H_
