#include <stdlib.h>
#include <drivers/sd.h>
#include <drivers/io.h>
#include <drivers/platform.h>
#include <memory/malloc.h>
#include <drivers/mailbox.h>

// ALL SECRTIONS FROM JESD84-A43

// Section 7.3
// INITIALIZATION SEQUENCE
// CMD0 -> IDLE STATE
// CMD1 -> READY or INACTIVE (cant access sector mode)
// CMD2 -> ID STATE or READY (card looses bus)
// CMD3 -> STAND BY
// CMD40 -> WAIT IRQ

// Section 7.3.3
// ACCESS MODE VALIDATION
// CMD1 -> OCR register
// tells if card has sector addressing

// Section 7.3.5
// CARD IDENTIFICATION PROCESS
// CMD2 -> CID

// Section 7.4
// INTERRUPT MODE
// 

// Section 7.9.2
// COMMAND FORMAT
// 47,      start bit, always 0
// 46,      transmission bit, always 1
// 45:40,   6 command index bits
// 39:8,    32 argument bits
// 7:1,     CRC7
// 0,       end bit, always 0

// RESPONSE BIT TYPES
// _NONE = 0,
// _136BITS = 1,
// _48BITS = 2,
// _48BITS_USING_BUSY = 3,

#define SD_BASE     (IO_BASE + 0x00300000)

#define EMMC_ARG2           ((SD_BASE+0x00))
#define EMMC_BLKSIZECNT     ((SD_BASE+0x04))
#define EMMC_ARG1           ((SD_BASE+0x08))
#define EMMC_CMDTM          ((SD_BASE+0x0C))
#define EMMC_RESP0          ((SD_BASE+0x10))
#define EMMC_RESP1          ((SD_BASE+0x14))
#define EMMC_RESP2          ((SD_BASE+0x18))
#define EMMC_RESP3          ((SD_BASE+0x1C))
#define EMMC_DATA           ((SD_BASE+0x20))
#define EMMC_STATUS         ((SD_BASE+0x24))
#define EMMC_CONTROL0       ((SD_BASE+0x28))
#define EMMC_CONTROL1       ((SD_BASE+0x2C))
#define EMMC_INTERRUPT      ((SD_BASE+0x30))
#define EMMC_IRPT_MASK      ((SD_BASE+0x34))
#define EMMC_IRPT_EN        ((SD_BASE+0x38))
#define EMMC_CONTROL2       ((SD_BASE+0x3C))
#define EMMC_HWCAP0         ((SD_BASE+0x40))
#define EMMC_HWCAP1         ((SD_BASE+0x44))
#define EMMC_HWMAXAMP0      ((SD_BASE+0x48))
#define EMMC_FORCE_IRPT     ((SD_BASE+0x50))
#define EMMC_DMA_STATUS     ((SD_BASE+0x54))
#define EMMC_BOOT_TIMEOUT   ((SD_BASE+0x70))
#define EMMC_DBG_SEL        ((SD_BASE+0x74))
#define EMMC_EXRDFIFO_CFG   ((SD_BASE+0x80))
#define EMMC_EXRDFIFO_EN    ((SD_BASE+0x84))
#define EMMC_TUNE_STEP      ((SD_BASE+0x88))
#define EMMC_TUNE_STEPS_STD ((SD_BASE+0x8c))
#define EMMC_TUNE_STEPS_DDR ((SD_BASE+0x90))
#define EMMC_BUS_CTRL       ((SD_BASE+0xe0))
#define EMMC_SPI_INT_SPT    ((SD_BASE+0xf0))
#define EMMC_SLOTISR_VER    ((SD_BASE+0xFC))

// EMMC_BLKSIZECNT
#define EMMC_BLKSIZECNT_BLKSIZE         0x00000fff
#define EMMC_BLKSIZECNT_SDMA_BLKSIZE    0x00007000
#define EMMC_BLKSIZECNT_BLKSIZE_MS1     0x00008000
#define EMMC_BLKSIZECNT_BLKCNT          0xffff0000

// EMMC_CMDTM
#define EMMC_CMDTM_TM_DMA_EN        0x00000001
#define EMMC_CMDTM_TM_BLKCNT_EN     0x00000002
#define EMMC_CMDTM_TM_AUTO_CMD_EN   0x0000000c
#define EMMC_CMDTM_TM_DAT_DIR       0x00000010
#define EMMC_CMDTM_TM_MULTI_BLOCK   0x00000020
#define EMMC_CMDTM_CMD_RSPNS_TYPE   0x00030000
#define EMMC_CMDTM_CMD_CRCCHK_EN    0x00080000
#define EMMC_CMDTM_CMD_IXCHK_EN     0x00100000
#define EMMC_CMDTM_CMD_ISDATA       0x00200000
#define EMMC_CMDTM_CMD_TYPE         0x00c00000
#define EMMC_CMDTM_CMD_INDEX        0x3f000000

#define CMD_FLAGS 0x00000000 //| (EMMC_CMDTM_CMD_IXCHK_EN | EMMC_CMDTM_CMD_CRCCHK_EN)

#define CMD0  (CMD_FLAGS | 0x00000000)                 // GO_IDLE_STATE
#define CMD1  (CMD_FLAGS | (1 << 24) | (1 << 17))      // SEND_OP_COND
#define CMD2  (CMD_FLAGS | (2 << 24) | (1 << 16))      // ALL_SEND_CID
#define CMD3  (CMD_FLAGS | (3 << 24) | (1 << 17))      // SET_RELATIVE_ADDR
#define CMD12 (CMD_FLAGS | (12 << 24) | (1 << 17))    // STOP_TRANSMISSION
#define CMD13 (CMD_FLAGS | (13 << 24) | (1 << 17))    // SEND_STATUS
#define CMD17 (CMD_FLAGS | (17 << 24) | (1 << 17))    // READ_SINGLE_BLOCK
#define CMD18 (CMD_FLAGS | (18 << 24) | (1 << 17))    // READ_MULTIPLE_BLOCK
#define CMD23 (CMD_FLAGS | (23 << 24) | (1 << 17))    // SET_BLOCK_COUNT
#define CMD24 (CMD_FLAGS | (24 << 24) | (1 << 17))    // WRITE_SINGLE_BLOCK
#define CMD25 (CMD_FLAGS | (25 << 24) | (1 << 17))    // WRITE_MULTIPLE_BLOCK
#define CMD55 (CMD_FLAGS | (55 << 24))

#define CMD_NEED_APP 0x80000000

#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

// COMMANDs
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x12220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010|CMD_NEED_APP)

// EMMC_STATUS
#define EMMC_STATUS_CMD_INHIBIT     0x00000001
#define EMMC_STATUS_DAT_INHIBIT     0x00000002
#define EMMC_STATUS_DAT_ACTIVE      0x00000004
#define EMMC_STATUS_RETUNING_REQ    0x00000008
#define EMMC_STATUS_WRITE_TRANSFER1 0x00000100
#define EMMC_STATUS_READ_TRANSFER   0x00000200
#define EMMC_STATUS_NEW_WRITE_DATA  0x00000400
#define EMMC_STATUS_NEW_READ_DATA   0x00000800
#define EMMC_STATUS_CARD_INSERT     0x00010000
#define EMMC_STATUS_CARD_STABLE     0x00020000
#define EMMC_STATUS_CARD_DETECT     0x00040000
#define EMMC_STATUS_WRT_PROTECT     0x00080000
#define EMMC_STATUS_DAT_LEVEL0      0x00f00000
#define EMMC_STATUS_CMD_LEVEL       0x01000000
#define EMMC_STATUS_DAT_LEVEL1      0x1e000000

// EMMC_CONTROL0
#define EMMC_CONTROL0_HCTL_LED      0x00000001
#define EMMC_CONTROL0_HCTL_DWIDTH   0x00000002
#define EMMC_CONTROL0_HCTL_HS_EN    0x00000004
#define EMMC_CONTROL0_HCTL_DMA      0x00000018
#define EMMC_CONTROL0_HCTL_8BIT     0x00000020
#define EMMC_CONTROL0_HCTL_CRDDET   0x00000040
#define EMMC_CONTROL0_HCTL_CRDDET_S 0x00000080
#define EMMC_CONTROL0_PWCTL_ON      0x00000100
#define EMMC_CONTROL0_PWCTL_SDVOLTS 0x00000e00
#define EMMC_CONTROL0_PWCTL_HWRST   0x00001000
#define EMMC_CONTROL0_GAP_STOP      0x00010000
#define EMMC_CONTROL0_GAP_RESTART   0x00020000
#define EMMC_CONTROL0_READWAIT_EN   0x00040000
#define EMMC_CONTROL0_GAP_IEN       0x00080000
#define EMMC_CONTROL0_SPI_MODE      0x00100000
#define EMMC_CONTROL0_BOOT_EN       0x00200000
#define EMMC_CONTROL0_ALT_BOOT_EN   0x00400000
#define EMMC_CONTROL0_WAKE_ONINT_EN 0x01000000
#define EMMC_CONTROL0_WAKE_ONINS_EN 0x02000000
#define EMMC_CONTROL0_WAKE_ONREM_EN 0x04000000

// EMMC_CONTROL1
#define EMMC_CONTROL1_CLK_INTLEN    0x00000001
#define EMMC_CONTROL1_CLK_STABLE    0x00000002
#define EMMC_CONTROL1_CLK_EN        0x00000004
#define EMMC_CONTROL1_CLK_GENSEL    0x00000020
#define EMMC_CONTROL1_CLK_FREQ_MS2  0x000000c0
#define EMMC_CONTROL1_CLK_FREQ8     0x0000ff00
#define EMMC_CONTROL1_DATA_TOUNIT   0x000f0000
#define EMMC_CONTROL1_SRST_HC       0x01000000
#define EMMC_CONTROL1_SRST_CMD      0x02000000
#define EMMC_CONTROL1_SRST_DATA     0x04000000

// EMMC_IRPT
#define EMMC_IRPT_CMD_DONE          0x00000001
#define EMMC_IRPT_DATA_DONE         0x00000002
#define EMMC_IRPT_BLOCK_GAP         0x00000004
#define EMMC_IRPT_DMA               0x00000008
#define EMMC_IRPT_WRITE_RDY         0x00000010
#define EMMC_IRPT_READ_RDY          0x00000020
#define EMMC_IRPT_CARD_IN           0x00000040
#define EMMC_IRPT_CARD_OUT          0x00000080
#define EMMC_IRPT_CARD              0x00000100
#define EMMC_IRPT_INT_A             0x00000200
#define EMMC_IRPT_INT_B             0x00000400
#define EMMC_IRPT_INT_C             0x00000800
#define EMMC_IRPT_RETUNE            0x00001000
#define EMMC_IRPT_BOOTACK           0x00002000
#define EMMC_IRPT_ENDBOOT           0x00004000
#define EMMC_IRPT_ERR               0x00008000
#define EMMC_IRPT_CTO_ERR           0x00010000
#define EMMC_IRPT_CCRC_ERR          0x00020000
#define EMMC_IRPT_CEND_ERR          0x00040000
#define EMMC_IRPT_CBAD_ERR          0x00080000
#define EMMC_IRPT_DTO_ERR           0x00100000
#define EMMC_IRPT_DCRC_ERR          0x00200000
#define EMMC_IRPT_DEND_ERR          0x00400000
#define EMMC_IRPT_SDOFF_ERR         0x00800000
#define EMMC_IRPT_ACMD_ERR          0x01000000
#define EMMC_IRPT_ADMA_ERR          0x02000000
#define EMMC_IRPT_TUNE_ERR          0x04000000
#define EMMC_IRPT_DMA_ERR           0x10000000
#define EMMC_IRPT_ATA_ERR           0x20000000
#define EMMC_IRPT_OEM_ERR           0xc0000000

// EMMC_CONTROL2
#define EMMC_CONTROL2_ACNOX_ERR     0x00000001
#define EMMC_CONTROL2_ACTO_ERR      0x00000002
#define EMMC_CONTROL2_ACCRC_ERR     0x00000004
#define EMMC_CONTROL2_ACEND_ERR     0x00000008
#define EMMC_CONTROL2_ACBAD_ERR     0x00000010
#define EMMC_CONTROL2_NOTC12_ERR    0x00000080
#define EMMC_CONTROL2_UHSMODE       0x00070000
#define EMMC_CONTROL2_SIGTYPE       0x00080000
#define EMMC_CONTROL2_DRVTYPE       0x00300000
#define EMMC_CONTROL2_TUNEON        0x00400000
#define EMMC_CONTROL2_TUNED         0x00800000
#define EMMC_CONTROL2_EN_AINT       0x40000000
#define EMMC_CONTROL2_EN_PSV        0x80000000

// EMMC_HWCAP0
#define EMMC_HWCAP0_TCLKFREQ        0x0000003f
#define EMMC_HWCAP0_TCLKUNIT        0x00000080
#define EMMC_HWCAP0_BASEMHZ         0x0000ff00
#define EMMC_HWCAP0_MAXLEN          0x00030000
#define EMMC_HWCAP0_XMEDBUS         0x00040000
#define EMMC_HWCAP0_ADMA2           0x00080000
#define EMMC_HWCAP0_HS              0x00200000
#define EMMC_HWCAP0_SDMA            0x00400000
#define EMMC_HWCAP0_RESUME          0x00800000
#define EMMC_HWCAP0_V3_3            0x01000000
#define EMMC_HWCAP0_V3_0            0x02000000
#define EMMC_HWCAP0_V1_8            0x04000000
#define EMMC_HWCAP0_BUS64           0x10000000
#define EMMC_HWCAP0_AINT            0x20000000
#define EMMC_HWCAP0_SLOT_TYPE       0xc0000000

// EMMC_HWCAP1
#define EMMC_HWCAP1_SDR50           0x00000001
#define EMMC_HWCAP1_SDR104          0x00000002
#define EMMC_HWCAP1_DDR50           0x00000004
#define EMMC_HWCAP1_DRV18_TYPEA     0x00000010
#define EMMC_HWCAP1_DRV18_TYPEC     0x00000020
#define EMMC_HWCAP1_DRV18_TYPED     0x00000040
#define EMMC_HWCAP1_RETUNE_TMR      0x00000f00
#define EMMC_HWCAP1_SDR50_TUNE      0x00002000
#define EMMC_HWCAP1_DATA_RETUNE     0x0000c000
#define EMMC_HWCAP1_MULTIPLIER      0x00ff0000
#define EMMC_HWCAP1_SPI_MODE        0x01000000
#define EMMC_HWCAP1_SPI_BLOCKMODE   0x02000000

// EMMC_HWMAXAMP0
#define EMMC_HWMAXAMP0_AMP_33V      0x000000ff
#define EMMC_HWMAXAMP0_AMP_30V      0x0000ff00
#define EMMC_HWMAXAMP0_AMP_18V      0x00ff0000

// EMMC_FORCE_IRPT
#define EMMC_FORCE_IRPT_CMD_DONE    0x00000001
#define EMMC_FORCE_IRPT_DATA_DONE   0x00000002
#define EMMC_FORCE_IRPT_BLOCK_GAP   0x00000004
#define EMMC_FORCE_IRPT_DMA         0x00000008
#define EMMC_FORCE_IRPT_WRITE_RDY   0x00000010
#define EMMC_FORCE_IRPT_READ_RDY    0x00000020
#define EMMC_FORCE_IRPT_CARD_IN     0x00000040
#define EMMC_FORCE_IRPT_CARD_OUT    0x00000080
#define EMMC_FORCE_IRPT_CTO_ERR     0x00010000
#define EMMC_FORCE_IRPT_CCRC_ERR    0x00020000
#define EMMC_FORCE_IRPT_CEND_ERR    0x00040000
#define EMMC_FORCE_IRPT_CBAD_ERR    0x00080000
#define EMMC_FORCE_IRPT_DTO_ERR     0x00100000
#define EMMC_FORCE_IRPT_DCRC_ERR    0x00200000
#define EMMC_FORCE_IRPT_DEND_ERR    0x00400000
#define EMMC_FORCE_IRPT_SDOFF_ERR   0x00800000
#define EMMC_FORCE_IRPT_ACMD_ERR    0x01000000
#define EMMC_FORCE_IRPT_ADMA_ERR    0x02000000
#define EMMC_FORCE_IRPT_TUNE_ERR    0x04000000
#define EMMC_FORCE_IRPT_DMA_ERR     0x10000000
#define EMMC_FORCE_IRPT_ATA_ERR     0x20000000
#define EMMC_FORCE_IRPT_OEM_ERR     0xc0000000

// EMMC_DMA_STATUS
#define EMMC_DMA_STATUS_ERR_AT      0x00000003
#define EMMC_DMA_STATUS_LEN_NOMATCH 0x00000004

// EMMC_BOOT_TIMEOUT
#define EMMC_BOOT_TIMEOUT_TIMEOUT   0xffffffff

// EMMC_DBG_SEL
#define EMMC_DBG_SEL_SELECT         0x00000001

// EMMC_EXRDFIFO_CFG
#define EMMC_EXRDFIFO_CFG_RD_THRSH  0x00000007

// EMMC_EXRDFIFO_EN
#define EMMC_EXRDFIFO_EN_ENABLE     0x00000001

// EMMC_TUNE_STEP
#define EMMC_TUNE_STEP_DELAY        0x00000007

// EMMC_TUNE_STEPS_STD
#define EMMC_TUNE_STEPS_STD_STEPS   0x0000003f

// EMMC_TUNE_STEPS_DDR
#define EMMC_TUNE_STEPS_DDR_STEPS   0x0000003f

// EMMC_BUS_CTRL
#define EMMC_BUS_CTRL_CLK_PINS      0x00000007
#define EMMC_BUS_CTRL_IRQ_PINS      0x00000038
#define EMMC_BUS_CTRL_BUS_WIDTH     0x00007f00
#define EMMC_BUS_CTRL_IRQSEL        0x00700000
#define EMMC_BUS_CTRL_BE_PWR        0x7f000000

// EMMC_SPI_INT_SPT
#define EMMC_SPI_INT_SPT_SELECT     0x000000ff

// EMMC_SLOTISR_VER
#define EMMC_SLOTISR_VER_SLOT_STATUS    0x000000ff
#define EMMC_SLOTISR_VER_SDVERSION      0x00ff0000
#define EMMC_SLOTISR_VER_VENDOR         0xff000000

#define SD_STATE_IDLE       0
#define SD_STATE_READY      1
#define SD_STATE_IDENT      2
#define SD_STATE_STDBY      3
#define SD_STATE_TRAN       4
#define SD_STATE_DATA       5
#define SD_STATE_RCV        6
#define SD_STATE_PRG        7
#define SD_STATE_DIS        8
#define SD_STATE_BTST       9
#define SD_STATE_SLP        10


#define SR_READ_AVAILABLE   0x00000800
#define SR_WRITE_AVAILABLE  0x00000400
#define SR_DAT_INHIBIT      0x00000002
#define SR_CMD_INHIBIT      0x00000001
#define SR_APP_CMD          0x00000020

#define INT_DATA_TIMEOUT    0x00100000
#define INT_CMD_TIMEOUT     0x00010000
#define INT_READ_RDY        0x00000020
#define INT_WRITE_RDY       0x00000010
#define INT_DATA_DONE       0x00000002
#define INT_CMD_DONE        0x00000001

#define INT_ERROR_MASK      0xF7FE8000

#define C0_SPI_MODE_EN      0x00100000
#define C0_HCTL_HS_EN       0x00000004
#define C0_HCTL_DWITDH      0x00000002

#define C1_SRST_DATA        0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_DIS       0x000f0000
#define C1_TOUNIT_MAX       0x000e0000
#define C1_CLK_GENSEL       0x00000020
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001

#define HOST_SPEC_NUM       0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

#define SCR_SD_BUS_WIDTH_4  0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51ff8000
/*
typedef union CID {
    struct CID_fields {
        unsigned NA:1;      // Always 1
        unsigned CRC:7;     // CRC7 checksum
        struct MDT {
            unsigned m:4;
            unsigned y:4;
        } MDT;            // 2x4 hex digits month, year manufacturing date
        uint32_t PSN:32;    // A 32 bit unsigned binary integer
        struct PRV {
            unsigned n:4;
            unsigned m:4;
        } PRV;           // 2x4 bit BCD revision number
        char PNM[6];    // 6 ASCII character product name
        uint8_t OID:8;     // Card OEM Identification
        unsigned CBX:2;    // Device Type
        unsigned RES:6;    // Reserved
        uint8_t MID:8;     // Manufacturer Identification
    } fields;
    uint8_t val[16];
} CID;
CID sd_cid;

typedef struct CSD {
    unsigned CSD_STRUCTURE:2;           // CSD Version
    unsigned SPEC_VERS:4;               // System Spec version
    unsigned RES1:2;                    // Reserved
    struct TAAC {
        unsigned RES:1;
        unsigned MULT:4;
        unsigned TIME:3;
    } TAAC;
    unsigned NSAC:8;
    struct TRAN_SPEED {
        unsigned RES:1;
        unsigned MULT:4;
        unsigned FREQ:3;
    } TRAN_SPEED;
    unsigned CCC:12;
    unsigned READ_BL_LEN:4;
    unsigned READ_BL_PARTIAL:1;
    unsigned WRITE_BLK_MISALIGN:1;
    unsigned READ_BLK_MISALIGN:1;
    unsigned DSR_IMPL:1;
    unsigned RES2:2;
    unsigned C_SIZE:12;
    unsigned VDD_R_CURR_MIN:3;
    unsigned VDD_R_CURR_MAX:3;
    unsigned VDD_W_CURR_MIN:3;
    unsigned VDD_W_CURR_MAX:3;
    unsigned C_SIZE_MULT:3;
    unsigned ERASE_GRP_SIZE:5;
    unsigned ERASE_FRP_MULT:5;
    unsigned WP_GRP_SIZE:5;
    unsigned WP_GRP_ENABLE:1;
    unsigned DEFAULT_ECC:2;
    unsigned R2W_FACTOR:3;
    unsigned WRITE_BL_LEN:4;
    unsigned WRITE_BL_PARTIAL:1;
    unsigned RES3:4;
    unsigned CONTENT_PROT_APP:1;
    unsigned FILE_FORMAT_GRP:1;
    unsigned COPY:1;
    unsigned PERM_WRITE_PROTECT:1;
    unsigned TMP_WRITE_PROTECT:1;
    unsigned FILE_FORMAT:2;
    unsigned ECC:2;
    unsigned CRC:7;
    unsigned NA:1;
} CSD;
*/
struct emmc_block_dev *emmc_dev;

/*
typedef union OCR {
    struct OCR_fields {
        unsigned RES1:7;
        unsigned LV:1;
        unsigned MV:7;
        unsigned HV:9;
        unsigned RES2:5;
        unsigned ACCESS:2;
        unsigned BUSY:1;
    } fields;
    uint32_t val;
} OCR;
OCR sd_ocr;
typedef union RESPONSE {
    struct R1 {
        unsigned END:1;
        unsigned CRC7:7;
        struct R1_STATUS {
            unsigned RES1:5;
            unsigned ACMD:1;
            unsigned RES2:1;
            unsigned DRDY:1;
            unsigned STATE:4;
            unsigned ER_RST:1;
            unsigned RES3:1;
            unsigned WP_ER_SKP:1;
            unsigned OVRW:1;
            unsigned OVRN:1;
            unsigned UNRN:1;
            unsigned ERR:1;
            unsigned CC_ERR:1;
            unsigned ECC_ERR:1;
            unsigned ILL_CMD:1;
            unsigned CRC_ERR:1;
            unsigned LOCK_FAIL:1;
            unsigned SD_LCKD:1;
            unsigned WP_V:1;
            unsigned ER_PAR:1;
            unsigned ER_SEQ_ERR:1;
            unsigned BLK_LEN_ERR:1;
            unsigned ADDR_MIS:1;
            unsigned ADDR_RNG:1;
        } STATUS;
        unsigned CMD:6;
        unsigned TX:1;
        unsigned START:1;
    } R1;
    struct R2 {
        CID CID;
        unsigned CHK:6;
        unsigned TX:1;
        unsigned START:1;
    } R2;
    struct R3 {
        unsigned END:1;
        unsigned CRC7:7;
        unsigned OCR:32;
        unsigned CHK:6;
        unsigned TX:1;
        unsigned START:1;
    } R3;
    struct R4 {
        unsigned END:1;
        unsigned CRC7:7;
        struct R4_ARG {
            unsigned RRC:8;
            unsigned REGA:7;
            unsigned STAT:1;
            unsigned RCA:16;
        } ARG;
        unsigned CMD39:6;
        unsigned TX:1;
        unsigned START:1;
    } R4;
    struct R5 {
        unsigned END:1;
        unsigned CRC7:7;
        struct R5_ARG {
            unsigned NA:16;
            unsigned RCA:16;
        } ARG;
        unsigned CMD40:6;
        unsigned TX:1;
        unsigned START:1;
    } R5;
    uint8_t val[17];
} RESPONSE;
RESPONSE sd_res;
*/

uint8_t crc_table[256];

void make_crc_table(void)
{
    int i = 0, j = 1;
    for (i = 0; i < 256; i++) {
        crc_table[i] = (i & 0x80) ? i ^ 0x89 : i;
        for (j = 1; j < 8; j++) {
            crc_table[i] <<= 1;
            if (crc_table[i] & 0x80) {crc_table[i] ^= 0x89;}
        }
    }
}

uint8_t crc7(uint8_t *input, uint32_t input_len)
{
    uint8_t crc = 0;
    for (int i = 0; i < input_len; i++) {
        crc = crc_table[(crc << 1) ^ input[i]];
    }
    return (crc << 1) | 1;
}

static inline void make_cmd(uint8_t *__cmd, uint8_t __idx, uint32_t __arg)
{
    memset(__cmd, 0, 6);\
    __cmd[0] = (1 << 6) | __idx;\
    __cmd[1] = (__arg >> 0x18) & 0xff;\
    __cmd[2] = (__arg >> 0x10) & 0xff;\
    __cmd[3] = (__arg >> 0x08) & 0xff;\
    __cmd[4] = (__arg >> 0x00) & 0xff;\
    __cmd[5] = crc7(__cmd, 5);
}

err_t sd_status(unsigned int mask)
{
    int cnt = 1000000;
    while (((mmio_read(EMMC_STATUS)) & mask) && !((mmio_read(EMMC_INTERRUPT)) & INT_ERROR_MASK) && cnt--) {
        udelay(1000);
    }
    return (cnt <= 0 || ((mmio_read(EMMC_INTERRUPT)) & INT_ERROR_MASK)) ? E_NOT_READY : E_NOERR;
}

static err_t sd_int(unsigned int mask)
{
    unsigned int r, m = mask | INT_ERROR_MASK;

    int cnt = 1000000;
    while (!((mmio_read(EMMC_INTERRUPT)) & m) && cnt--) {
        udelay(1000);
    }
    r = mmio_read(EMMC_INTERRUPT);
    if (cnt <= 0 || (r & EMMC_IRPT_CTO_ERR) || (r & EMMC_IRPT_DTO_ERR)) {
        mmio_write(EMMC_INTERRUPT, r);
        mmio_write(EMMC_INTERRUPT, mask);
        printk("\x1b[1;31mSD: COMMAND OR DATA TIMEOUT %x\x1b[1;0m\n", r);
        return E_TIMEOUT;
    } else if (r & INT_ERROR_MASK) {
        mmio_write(EMMC_INTERRUPT, r);
        mmio_write(EMMC_INTERRUPT, mask);
        printk("\x1b[1;31mSD: INTERRUPT ERROR %x\x1b[1;0m\n", r);
        return E_NOT_READY;
    }
    mmio_write(EMMC_INTERRUPT, mask);
    return E_NOERR;
}

static uint32_t sd_cmd(unsigned int code, unsigned int arg) {
    uint32_t r = 0;
    emmc_dev->last_error = E_NOERR;

    if (code & CMD_NEED_APP) {
        r = sd_cmd(CMD55|(emmc_dev->card_rca?CMD_RSPNS_48:0), emmc_dev->card_rca);
        udelay(100000);
        code &= ~CMD_NEED_APP;
    }

    if (sd_status(EMMC_STATUS_CMD_INHIBIT)) {
        printk("\x1b[1;31mSD: EMMC busy\x1b[1;0m\n");
        emmc_dev->last_error = E_TIMEOUT;
        return 0;
    }

    printk("SD: Sending command %x arg %x\n", code, arg);
    mmio_write(EMMC_INTERRUPT, mmio_read(EMMC_INTERRUPT));
    mmio_write(EMMC_ARG1, arg);
    mmio_write(EMMC_CMDTM, code);

    if (code & (1 << 24)) {
        udelay(1000000);
    } else {
        udelay(100000);
    }

    if((r = sd_int(INT_CMD_DONE))) {
        printk("\x1b[1;31mSD: failed to send EMMC command\x1b[1;0m\n");
        emmc_dev->last_error = r;
        return 0;
    }
    r = emmc_dev->last_r0 = mmio_read(EMMC_RESP0);
    emmc_dev->last_r1 = mmio_read(EMMC_RESP1);
    emmc_dev->last_r2 = mmio_read(EMMC_RESP2);
    emmc_dev->last_r3 = mmio_read(EMMC_RESP3); 
    if(code==CMD_GO_IDLE || code==CMD_APP_CMD) {
        printk("\t\x1b[1;33mCASE SIMPLE\x1b[1;0m\n");
        return 0;
    } else if (code==(CMD_APP_CMD|CMD_RSPNS_48)) {
        printk("\t\x1b[1;33mCASE APP CMD\x1b[1;0m");
        return r & SR_APP_CMD;
    } else if (code==CMD_SEND_OP_COND) {
        printk("\t\x1b[1;33mCASE OP_COND\x1b[1;0m\n");
        return r;
    } else if(code==CMD_SEND_IF_COND)  {
        printk("\t\x1b[1;33mCASE IF_COND\x1b[1;0m\n");
        return r==arg? E_NOERR : E_NOT_READY;
    } else if(code==CMD_ALL_SEND_CID) {
        printk("\t\x1b[1;33mCASE CID\x1b[1;0m\n");
        r |= mmio_read(EMMC_RESP1);
        r |= mmio_read(EMMC_RESP2);
        r |= mmio_read(EMMC_RESP3); 
        return r;
    } else if(code==CMD_SEND_REL_ADDR) {
        printk("\t\x1b[1;33mCASE RCA\x1b[1;0m\n");
        emmc_dev->last_error=(((r & 0x1fff))|((r & 0x2000)<<6)|((r&  0x4000)<<8)|((r & 0x8000)<<8)) & CMD_ERRORS_MASK;
        return r & CMD_RCA_MASK;
    }
    printk("\t\x1b[1;33mCASE ERROR\x1b[1;0m\n");
    return r; // & CMD_ERRORS_MASK
}

static err_t sd_clk(unsigned int f)
{
    unsigned int d, c, x, s = 32, h = 0;

    c = (sd_get_base_clock_hz())/f;
    if (c % f) {
        c--;
    }

    int cnt = 100000;

    while (((mmio_read(EMMC_STATUS)) & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) && cnt--) {
        udelay(1000);
    }
    
    if (cnt <= 0) {
        printk("\x1b[1;31mSD: timeout waiting for inhibit flag\x1b[1;0m\n");
        return E_TIMEOUT;
    }

    sd_set_clock_state(0);

    mmio_write(EMMC_CONTROL1, mmio_read(EMMC_CONTROL1) & ~C1_CLK_EN);
    udelay(10000);
    x = c - 1;
    if (!x) {
        s = 0;
    } else {
        if (!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
        if (!(x & 0xff000000u)) { x <<= 8; s -= 8; }
        if (!(x & 0xf0000000u)) { x <<= 4; x -= 4; }
        if (!(x & 0xC0000000u)) { x <<= 2; x -= 2; }
        if (!(x & 0x80000000u)) { x <<= 1; x -= 1; }
        if (s > 0) s--;
        if (s > 7) s = 7;
    }
    if (emmc_dev->sd_hv > HOST_SPEC_V2) {
        d = c;
    } else {
        d = (1 << s);
    }
    if (d <= 2) {
        d = 2;
        s = 0;
    }

    printk("sd_clk divisor %d, shift %d\n", d, s);

    
    h = (d & 0x300) >> 2;
    d = (((d & 0x0ff) << 8) | h);
    mmio_write(EMMC_CONTROL1, (mmio_read(EMMC_CONTROL1) & 0xffff003f) | d);
    udelay(10000);
    sd_set_clock_state(1);
    mmio_write(EMMC_CONTROL1, mmio_read(EMMC_CONTROL1) | C1_CLK_EN);
    udelay(10000);
    cnt = 10000;
    while (!(mmio_read(EMMC_CONTROL1) & C1_CLK_STABLE) && cnt--) {
        udelay(10000);
    }
    if (cnt <= 0) {
        printk("\x1b[1;31mSD: failed to get stable clock\x1b[1;0m\n");
        return E_NOT_READY;
    }
    sd_get_clock_state();

    return E_NOERR;
}
/*
err_t sd_ioctl(unsigned int cmd, unsigned int arg)
{
    int r = 0;
    sd_err = E_NOERR;
    if (cmd & CMD_NEED_APP) {
        r = sd_ioctl(CMD_APP_CMD | (sd_rca ? CMD_RSPNS_48:0), sd_rca);
        if (sd_rca && !r) {
            sd_err = E_NOT_READY;
            return E_NOERR;
        }
        cmd &= ~CMD_NEED_APP;
    }
    if (sd_status(SR_CMD_INHIBIT)) {
        sd_err = E_TIMEOUT;
        return 0;
    }
    (*(unsigned int *)EMMC_ARG1) = arg;
    (*(unsigned int *)EMMC_CMDTM) = cmd;
    
    if (cmd == CMD_SEND_OP_COND) udelay(1000000);
    else if (cmd == CMD_SEND_IF_COND || cmd == CMD_APP_CMD) udelay(100000);

    if ((r = sd_int(INT_CMD_DONE))) {
        sd_err = r;
        return E_NOERR;
    }
    r = chip_read(EMMC_RESP0);
    
    if (cmd == CMD_GO_IDLE || cmd == CMD_APP_CMD) return E_NOERR;
    else if (cmd == (CMD_APP_CMD | CMD_RSPNS_48)) return r & SR_APP_CMD;
    else if (cmd == CMD_SEND_OP_COND) return r;
    else if (cmd == CMD_SEND_IF_COND) return r == arg ? E_NOERR : E_NOT_READY;
    else if (cmd == CMD_ALL_SEND_CID) {
        r |= chip_read(EMMC_RESP3);
        r |= chip_read(EMMC_RESP2);
        r |= chip_read(EMMC_RESP1);
        return r;
    } else if (cmd == CMD_SEND_REL_ADDR) {
        sd_err = (((r&0x1FFF))|((r&0x2000)<<6)|((r&0x4000)<<8)) & CMD_ERRORS_MASK;
        return r & CMD_RCA_MASK;
    }
    return r & CMD_ERRORS_MASK;
}
*/
size_t sd_read(uint8_t *buffer, size_t num, uint32_t lba)
{
    int r,c=0,d;

    if(num<1) {
        num=1;
    }

    printk("sd_readblock lba: %x, num: %d\n", lba, num);

    if(sd_status(SR_DAT_INHIBIT)) {
        emmc_dev->last_error = E_TIMEOUT;
        return 0;
    }

    unsigned int *buf = (unsigned int *)buffer;

    if(emmc_dev->scr[0] & SCR_SUPP_CCS) {
        if(num > 1 && (emmc_dev->scr[0] & SCR_SUPP_SET_BLKCNT)) {
            sd_cmd(CMD23,num);
            if(emmc_dev->last_error) {
                return 0;
            }
        }
        mmio_write(EMMC_BLKSIZECNT, (num << 16) | 512);
        sd_cmd((num == 1) ? CMD17 : CMD18, lba);
        if(emmc_dev->last_error){
            return 0;
        }
    } else {
        mmio_write(EMMC_BLKSIZECNT, (1 << 16) | 512);
    }
    while(c < num) {
        if(!(emmc_dev->scr[0] & SCR_SUPP_CCS)) {
            sd_cmd(CMD17, (lba + c) * 512);
            if(emmc_dev->last_error) {
                return 0;
            }
        }
        if((r = sd_int(INT_READ_RDY))) {
            printk("\x1b[1;31mSD: Timeout waiting for ready to read\x1b[1;0m\n");
            emmc_dev->last_error = r;
            return 0;
        }
        for(d = 0; d < 128; d ++) {
            buf[d] = mmio_read(EMMC_DATA);
        }
        c++;
        buf += 128;
    }
    if(num > 1 && !(emmc_dev->scr[0] & SCR_SUPP_SET_BLKCNT) && (emmc_dev->scr[0] & SCR_SUPP_CCS)) {
        sd_cmd(CMD12, 0);
    }
    return (emmc_dev->last_error != E_NOERR) || ((c != num)? 0 : (num * 512));
}

size_t sd_write(uint8_t *buffer, size_t num, uint32_t lba)
{
    int r,c=0,d;

    if(num<1) {
        num=1;
    }

    printk("sd_writeblock lba: %x, num: %d\n", lba, num);

    if(sd_status(SR_DAT_INHIBIT | SR_WRITE_AVAILABLE)) {
        emmc_dev->last_error = E_TIMEOUT;
        return 0;
    }

    unsigned int *buf = (unsigned int *)buffer;

    if(emmc_dev->scr[0] & SCR_SUPP_CCS) {
        if(num > 1 && (emmc_dev->scr[0] & SCR_SUPP_SET_BLKCNT)) {
            sd_cmd(CMD23, num);
            if(emmc_dev->last_error) {
                return 0;
            }
        }
        mmio_write(EMMC_BLKSIZECNT, (num << 16) | 512);
        sd_cmd((num == 1) ? CMD24 : CMD25, lba);
        if(emmc_dev->last_error) {
            return 0;
        }
    } else {
        mmio_write(EMMC_BLKSIZECNT, (1 << 16) | 512);
    }
    while(c < num) {
        if(!(emmc_dev->scr[0] & SCR_SUPP_CCS)) {
            sd_cmd(CMD24, (lba + c) * 512);
            if(emmc_dev->last_error) {
                return 0;
            }
        }
        if((r = sd_int(INT_WRITE_RDY))) {
            printk("\x1b[1;31mSD: Timeout waiting for ready to write\x1b[1;0m\n");
            emmc_dev->last_error = r;
            return 0;
        }
        for(d = 0; d < 128; d ++) {
            mmio_write(EMMC_DATA, buf[d]);
        }
        c++;
        buf += 128;
    }
    if((r = sd_int(INT_DATA_DONE))) {
        printk("\x1b[1;31mSD: Timeout waiting for data done\x1b[1;0m\n");
        emmc_dev->last_error = r;
        return 0;
    }
    if((num > 1) && !(emmc_dev->scr[0] & SCR_SUPP_SET_BLKCNT) && (emmc_dev->scr[0] & SCR_SUPP_CCS)) {
        sd_cmd(CMD12, 0);
    }

    return (emmc_dev->last_error != E_NOERR) || (c != num) ? 0 : (num * 512);
}

void sd_gpio(void) {
    int r;
    // GPIO_CD
    r = mmio_read(GPIO_GPFSEL4);
    r &= ~(7<<(7*3));
    mmio_write(GPIO_GPFSEL4, r);
    mmio_write(GPIO_GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPIO_GPPUDCLK1, (1 << 15));
    wait_cycles(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);
    r = mmio_read(GPIO_GPHEN1);
    r |= 1<<15;
    mmio_write(GPIO_GPHEN1, r);

    // GPIO_CLK, GPIO_CMD
    r = mmio_read(GPIO_GPFSEL4);
    r |= (7<<(8*3)) | (7<<(9*3));
    mmio_write(GPIO_GPFSEL4, r);
    mmio_write(GPIO_GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPIO_GPPUDCLK1, (1<<17));
    wait_cycles(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = mmio_read(GPIO_GPFSEL5);
    r |= (7<<(0*3)) | (7<<(1*3)) | (7<<(2*3)) | (7<<(3*3));
    mmio_write(GPIO_GPFSEL5, r);
    mmio_write(GPIO_GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPIO_GPPUDCLK1, (1<<18) | (1<<19) | (1<<20) | (1<<21));
    wait_cycles(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);
}

int sd_init(struct block_device *dev)
{
    uint64_t r,cnt,ccs=0;
/*
    // GPIO_CD
    r = mmio_read(GPIO_GPFSEL4);
    r &= ~(7<<(7*3));
    mmio_write(GPIO_GPFSEL4, r);
    mmio_write(GPIO_GPPUD, 2);
    udelay(150);
    mmio_write(GPIO_GPPUDCLK1, (1 << 15));
    udelay(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);
    r = mmio_read(GPIO_GPHEN1);
    r |= 1<<15;
    mmio_write(GPIO_GPHEN1, r);
*/
    // GPIO_CLK, GPIO_CMD
    r = mmio_read(GPIO_GPFSEL3);
    r |= (7<<(4*3)) | (7<<(5*3));
    mmio_write(GPIO_GPFSEL3, r);
    mmio_write(GPIO_GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPIO_GPPUDCLK1, (1<<3));
    wait_cycles(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = mmio_read(GPIO_GPFSEL3);
    r |= (7<<(6*3)) | (7<<(7*3)) | (7<<(8*3)) | (7<<(9*3));
    mmio_write(GPIO_GPFSEL3, r);
    mmio_write(GPIO_GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPIO_GPPUDCLK1, (1<<4) | (1<<5) | (1<<6) | (1<<7));
    wait_cycles(150);
    mmio_write(GPIO_GPPUD, 0);
    mmio_write(GPIO_GPPUDCLK1, 0);

    new(emmc_dev, 1, struct emmc_block_dev);

    //sd_gpio();

    //make_crc_table();

    uint32_t d = mmio_read(0xfe2000d0);
    printk("Default mode:       %d\n", d);
    d &= 0xfffffffc;
    d |= 0x2;
    mmio_write(0xfe2000d0, d);
    d = mmio_read(0xfe2000d0);
    printk("Changed to mode:    %d\n", d);

    // Reset the card.
    mmio_write(EMMC_CONTROL0, 0);
    mmio_write(EMMC_CONTROL1, 0);
    mmio_write(EMMC_CONTROL2, 0);
    
    mmio_write(EMMC_CONTROL1, C1_SRST_HC);
    cnt = 10000;
    do{
        udelay(10000);
    } while((mmio_read(EMMC_CONTROL1) & C1_SRST_HC) && cnt-- );
    if(cnt <= 0) {
        printk("\x1b[1;31mSD: failed to reset EMMC\x1b[1;0m\n");
        return E_NOT_READY;
    }
    printk("\x1b[1;32mSD: reset OK\x1b[1;0m\n");
    

    sd_disable_low_power();
    // Enable SD Bus Power VDD1 at 3.3V
	uint32_t control0 = mmio_read(EMMC_CONTROL0);
	control0 |= 0x0F << 8;
	mmio_write(EMMC_CONTROL0, control0);
	udelay(2000);

    emmc_dev->sd_hv = (mmio_read(EMMC_SLOTISR_VER) & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;

    mmio_write(EMMC_CONTROL1 , mmio_read(EMMC_CONTROL1) | C1_CLK_INTLEN | C1_TOUNIT_DIS);
    udelay(10000);
    // Set clock to setup frequency.
    if((r = sd_clk(400000))) {
        return r;
    }

    mmio_write(EMMC_INTERRUPT, 0x00000000);
    mmio_write(EMMC_IRPT_MASK, INT_ERROR_MASK | EMMC_IRPT_CMD_DONE);
    mmio_write(EMMC_IRPT_EN, INT_ERROR_MASK | EMMC_IRPT_CMD_DONE);

    // Send standby
    sd_cmd(CMD0,0);

    printk("\x1b[1;32mSD CARD IN IDENTIFICATION STATE\x1b[1;0m\n");

    // Send requirements command
    sd_cmd(CMD_SEND_IF_COND,0x000001AA);
    if(emmc_dev->last_error) return emmc_dev->last_error;
    cnt=6;
    r=0;
    while(!(r & ACMD41_CMD_COMPLETE) && cnt--) {
        udelay(400);
        r = sd_cmd(CMD_SEND_OP_COND,ACMD41_ARG_HC);
    }
    if(!(r & ACMD41_CMD_COMPLETE) || !cnt ) return E_NOT_READY;
    if(!(r & ACMD41_VOLTAGE)) return E_NOERR;
    if(r & ACMD41_CMD_CCS) ccs=SCR_SUPP_CCS;

    if (r & ACMD41_VOLTAGE) {
        printk("Full Voltage Support\n");
    }
    if ((r & 0x60000000) == 2<<29) {
        printk("Sector Support\n");
    } else if ((r & 0x60000000) == 0) {
        printk("Byte Support\n");
    }
    //printk("RESP: %x\n", r);

    printk("\x1b[1;32mSD CARD IN READY STATE\x1b[1;0m\n");

    sd_cmd(CMD2, 0);

    r = sd_cmd(CMD3, (1 << 16));
    //printk("RESP CMD3: %x\n", r);
    //if (sd_res.R1.STATUS.STATE != SD_STATE_IDENT) {
    //    printk("\x1b[1;31mSD: DEVICE FAILED READY SEQUENCE %x\x1b[1;0m\n", sd_res.R1.STATUS.STATE);
    //    return E_NOT_READY;
    //}

    //r = sd_cmd(CMD13, (1 << 16));
    //printk("RESP CMD13: %x\n", r);
    //if (sd_res.R1.STATUS.STATE != SD_STATE_STDBY) {
    //    printk("\x1b[1;31mSD: DEVICE FAILED READY SEQUENCE %x\x1b[1;0m\n", sd_res.R1.STATUS.STATE);
    //    return E_NOT_READY;
    //}

    //printk("\x1b[1;32mSD CARD IN STANDBY STATE\x1b[1;0m\n");

/*
    sd_scr[0] = sd_scr[1] = sd_rca = sd_err = 0;
    sd_cmd(CMD_GO_IDLE, 0);
    if(sd_err) {
        return sd_err;
    }

    sd_cmd(CMD_SEND_IF_COND, 0x000001AA);
    if(sd_err) {
        return sd_err;
    }
    cnt = 6;
    r = 0;
    while(!(r & ACMD41_CMD_COMPLETE) && cnt--) {
        udelay(400);
        r = sd_cmd(CMD_SEND_OP_COND, ACMD41_ARG_HC);
        printk("SD: CMD_SEND_OP_COND returned ");
        if(r & ACMD41_CMD_COMPLETE) {
            printk("COMPLETE ");
        }
        if(r & ACMD41_VOLTAGE) {
            printk("VOLTAGE ");
        }
        if(r & ACMD41_CMD_CCS) {
            printk("CCS ");
        }
        printk("%x%x\n", r>>32, r);
        if(sd_err != E_TIMEOUT && sd_err != E_NOERR) {
            printk("SD: EMMC ACMD41 returned error\n");
            return sd_err;
        }
    }
    if(!(r & ACMD41_CMD_COMPLETE) || !cnt ) {
        return E_TIMEOUT;
    }
    if(!(r & ACMD41_VOLTAGE)) {
        return E_NOT_READY;
    }
    if(r & ACMD41_CMD_CCS) {
        ccs = SCR_SUPP_CCS;
    }

    sd_cmd(CMD_ALL_SEND_CID, 0);

    sd_rca = sd_cmd(CMD_SEND_REL_ADDR, 0);
    printk("SD: CMD_SEND_REL_ADDR returned %x%x\n", sd_rca>>32, sd_rca);
    if(sd_err) {
        return sd_err;
    }

    if((r = sd_clk(25000000))) {
        return r;
    }

    sd_cmd(CMD_CARD_SELECT, sd_rca);
    if(sd_err) {
        return sd_err;
    }

    if(sd_status(SR_DAT_INHIBIT)) {
        return E_TIMEOUT;
    }
    mmio_write(EMMC_BLKSIZECNT, (1<<16) | 8);
    sd_cmd(CMD_SEND_SCR, 0);
    if(sd_err) {
        return sd_err;
    }
    if(sd_int(INT_READ_RDY)) {
        return E_TIMEOUT;
    }

    r = 0; 
    cnt = 100000; 
    while((r < 2) && cnt) {
        if(mmio_read(EMMC_STATUS) & SR_READ_AVAILABLE ) {
            sd_scr[r++] = mmio_read(EMMC_DATA);
        } else {
            udelay(1000);
        }
    }
    if(r != 2) {
        return E_TIMEOUT;
    }
    if(sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
        sd_cmd(CMD_SET_BUS_WIDTH, sd_rca | 2);
        if(sd_err) {
            return sd_err;
        }
        mmio_write(EMMC_CONTROL0, mmio_read(EMMC_CONTROL0) | C0_HCTL_DWITDH);
    }
    // add software flag
    printk("SD: supports ");
    if(sd_scr[0] & SCR_SUPP_SET_BLKCNT) {
        printk("SET_BLKCNT ");
    }
    if(ccs) {
        printk("CCS ");
    }
    printk("\n");
    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;
*/

	emmc_dev->bd.block_size = 512;
    emmc_dev->bd.supports_multiple_block_read = 1;
    emmc_dev->bd.supports_multiple_block_write = 1;
	emmc_dev->base_clock = 25000000;

    //if(dev != NULL) {
	//    *dev = ret->bd;
    //}
exit:
    return E_NOERR;
}