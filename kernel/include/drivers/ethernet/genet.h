#ifndef _GENET_H_
#define _GENET_H_

#include "../../stdlib.h"
#include "bcm54213pe.h"
#include "../io.h"
#include "netstruct.h"

#define GENET_SYS_REV_CTRL                      0x000
#define GENET_SYS_REV_MAJOR                     __BITS(27,24)
#define GENET_SYS_REV_MINOR                     __BITS(19,16)
#define GENET_SYS_PORT_CTRL                     0x004
#define GENET_SYS_PORT_MODE_EXT_GPHY            3
#define GENET_SYS_RBUF_FLUSH_CTRL               0x008
#define GENET_SYS_RBUF_FLUSH_RESET              __BIT(1)
#define GENET_SYS_TBUF_FLUSH_CTRL               0x00c
#define GENET_EXT_RGMII_OOB_CTRL                0x08c
#define GENET_EXT_RGMII_OOB_ID_MODE_DISABLE     __BIT(16)
#define GENET_EXT_RGMII_OOB_RGMII_MODE_EN       __BIT(6)
#define GENET_EXT_RGMII_OOB_OOB_DISABLE         __BIT(5)
#define GENET_EXT_RGMII_OOB_RGMII_LINK          __BIT(4)
#define GENET_INTRL2_CPU_STAT                   0x200
#define GENET_INTRL2_CPU_CLEAR                  0x208
#define GENET_INTRL2_CPU_STAT_MASK              0x20c
#define GENET_INTRL2_CPU_SET_MASK               0x210
#define GENET_INTRL2_CPU_CLEAR_MASK             0x214
#define GENET_IRQ_MDIO_ERROR                    __BIT(24)
#define GENET_IRQ_MDIO_DONE                     __BIT(23)
#define GENET_IRQ_TXDMA_DONE                    __BIT(16)
#define GENET_IRQ_RXDMA_DONE                    __BIT(13)
#define GENET_RBUF_CTRL                         0x300
#define GENET_RBUF_BAD_DIS                      __BIT(2)
#define GENET_RBUF_ALIGN_2B                     __BIT(1)
#define GENET_RBUF_64B_EN                       __BIT(0)
#define GENET_RBUF_TBUF_SIZE_CTRL               0x3b4
#define GENET_UMAC_CMD                          0x808
#define GENET_UMAC_CMD_LCL_LOOP_EN              __BIT(15)
#define GENET_UMAC_CMD_SW_RESET                 __BIT(13)
#define GENET_UMAC_CMD_PROMISC                  __BIT(4)
#define GENET_UMAC_CMD_SPEED                    __BITS(3,2)
#define GENET_UMAC_CMD_SPEED_10                 0
#define GENET_UMAC_CMD_SPEED_100                1
#define GENET_UMAC_CMD_SPEED_1000               2
#define GENET_UMAC_CMD_RXEN                     __BIT(1)
#define GENET_UMAC_CMD_TXEN                     __BIT(0)
#define GENET_UMAC_MAC0                         0x80c
#define GENET_UMAC_MAC1                         0x810
#define GENET_UMAC_MAX_FRAME_LEN                0x814
#define GENET_UMAC_TX_FLUSH                     0xb34
#define GENET_UMAC_MIB_CTRL                     0xd80
#define GENET_UMAC_MIB_RESET_TX                 __BIT(2)
#define GENET_UMAC_MIB_RESET_RUNT               __BIT(1)
#define GENET_UMAC_MIB_RESET_RX                 __BIT(0)
#define GENET_MDIO_CMD                          0xe14
#define GENET_MDIO_START_BUSY                   __BIT(29)
#define GENET_MDIO_READ                         __BIT(27)
#define GENET_MDIO_WRITE                        __BIT(26)
#define GENET_MDIO_PMD                          __BITS(25,21)
#define GENET_MDIO_REG                          __BITS(20,16)
#define GENET_UMAC_MDF_CTRL                     0xe50
#define GENET_UMAC_MDF_ADDR0(n)                 (0xe54 + (n) * 0x8)
#define GENET_UMAC_MDF_ADDR1(n)                 (0xe58 + (n) * 0x8)

#define GENET_DMA_DESC_COUNT                    256
#define GENET_DMA_DESC_SIZE                     12
#define GENET_DMA_DEFAULT_QUEUE                 16

#define GENET_DMA_RING_SIZE                     0x40
#define GENET_DMA_RINGS_SIZE                    (GENET_DMA_RING_SIZE * (GENET_DMA_DEFAULT_QUEUE + 1))

#define GENET_RX_BASE                           0x2000
#define GENET_TX_BASE                           0x4000

#define GENET_RX_DMA_RINGBASE(qid)              (GENET_RX_BASE + 0xc00 + GENET_DMA_RING_SIZE * (qid))
#define GENET_RX_DMA_WRITE_PTR_LO(qid)          (GENET_RX_DMA_RINGBASE(qid) + 0x00)
#define GENET_RX_DMA_WRITE_PTR_HI(qid)          (GENET_RX_DMA_RINGBASE(qid) + 0x04)
#define GENET_RX_DMA_PROD_INDEX(qid)            (GENET_RX_DMA_RINGBASE(qid) + 0x08)
#define GENET_RX_DMA_CONS_INDEX(qid)            (GENET_RX_DMA_RINGBASE(qid) + 0x0c)
#define GENET_RX_DMA_RING_BUF_SIZE(qid)         (GENET_RX_DMA_RINGBASE(qid) + 0x10)
#define GENET_RX_DMA_RING_BUF_SIZE_DESC_COUNT   __BITS(31,16)
#define GENET_RX_DMA_RING_BUF_SIZE_BUF_LENGTH   __BITS(15,0)
#define GENET_RX_DMA_START_ADDR_LO(qid)         (GENET_RX_DMA_RINGBASE(qid) + 0x14)
#define GENET_RX_DMA_START_ADDR_HI(qid)         (GENET_RX_DMA_RINGBASE(qid) + 0x18)
#define GENET_RX_DMA_END_ADDR_LO(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x1c)
#define GENET_RX_DMA_END_ADDR_HI(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x20)
#define GENET_RX_DMA_XON_XOFF_THRES(qid)        (GENET_RX_DMA_RINGBASE(qid) + 0x28)
#define GENET_RX_DMA_XON_XOFF_THRES_LO          __BITS(31,16)
#define GENET_RX_DMA_XON_XOFF_THRES_HI          __BITS(15,0)
#define GENET_RX_DMA_READ_PTR_LO(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x2c)
#define GENET_RX_DMA_READ_PTR_HI(qid)           (GENET_RX_DMA_RINGBASE(qid) + 0x30)

#define GENET_TX_DMA_RINGBASE(qid)              (GENET_TX_BASE + 0xc00 + GENET_DMA_RING_SIZE * (qid))
#define GENET_TX_DMA_READ_PTR_LO(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x00)
#define GENET_TX_DMA_READ_PTR_HI(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x04)
#define GENET_TX_DMA_CONS_INDEX(qid)            (GENET_TX_DMA_RINGBASE(qid) + 0x08)
#define GENET_TX_DMA_PROD_INDEX(qid)            (GENET_TX_DMA_RINGBASE(qid) + 0x0c)
#define GENET_TX_DMA_RING_BUF_SIZE(qid)         (GENET_TX_DMA_RINGBASE(qid) + 0x10)
#define GENET_TX_DMA_RING_BUF_SIZE_DESC_COUNT   __BITS(31,16)
#define GENET_TX_DMA_RING_BUF_SIZE_BUF_LENGTH   __BITS(15,0)
#define GENET_TX_DMA_START_ADDR_LO(qid)         (GENET_TX_DMA_RINGBASE(qid) + 0x14)
#define GENET_TX_DMA_START_ADDR_HI(qid)         (GENET_TX_DMA_RINGBASE(qid) + 0x18)
#define GENET_TX_DMA_END_ADDR_LO(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x1c)
#define GENET_TX_DMA_END_ADDR_HI(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x20)
#define GENET_TX_DMA_MBUF_DONE_THRES(qid)       (GENET_TX_DMA_RINGBASE(qid) + 0x24)
#define GENET_TX_DMA_FLOW_PERIOD(qid)           (GENET_TX_DMA_RINGBASE(qid) + 0x28)
#define GENET_TX_DMA_WRITE_PTR_LO(qid)          (GENET_TX_DMA_RINGBASE(qid) + 0x2c)
#define GENET_TX_DMA_WRITE_PTR_HI(qid)          (GENET_TX_DMA_RINGBASE(qid) + 0x30)

#define GENET_RX_DESC_STATUS(idx)               (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x00)
#define GENET_RX_DESC_STATUS_BUFLEN             __BITS(27,16)
#define GENET_RX_DESC_STATUS_OWN                __BIT(15)
#define GENET_RX_DESC_STATUS_EOP                __BIT(14)
#define GENET_RX_DESC_STATUS_SOP                __BIT(13)
#define GENET_RX_DESC_STATUS_RX_ERROR           __BIT(2)
#define GENET_RX_DESC_ADDRESS_LO(idx)           (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x04)
#define GENET_RX_DESC_ADDRESS_HI(idx)           (GENET_RX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x08)

#define GENET_TX_DESC_STATUS(idx)               (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x00)
#define GENET_TX_DESC_STATUS_BUFLEN             __BITS(27,16)
#define GENET_TX_DESC_STATUS_OWN                __BIT(15)
#define GENET_TX_DESC_STATUS_EOP                __BIT(14)
#define GENET_TX_DESC_STATUS_SOP                __BIT(13)
#define GENET_TX_DESC_STATUS_QTAG               __BITS(12,7)
#define GENET_TX_DESC_STATUS_CRC                __BIT(6)
#define GENET_TX_DESC_ADDRESS_LO(idx)           (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x04)
#define GENET_TX_DESC_ADDRESS_HI(idx)           (GENET_TX_BASE + GENET_DMA_DESC_SIZE * (idx) + 0x08)

#define GENET_RX_DMA_RING_CFG                   (GENET_RX_BASE + 0x1040 + 0x00)
#define GENET_RX_DMA_CTRL                       (GENET_RX_BASE + 0x1040 + 0x04)
#define GENET_RX_DMA_CTRL_RBUF_EN(qid)          __BIT((qid) + 1)
#define GENET_RX_DMA_CTRL_EN                    __BIT(0)
#define GENET_RX_SCB_BURST_SIZE                 (GENET_RX_BASE + 0x1040 + 0x0c)

#define GENET_TX_DMA_RING_CFG                   (GENET_TX_BASE + 0x1040 + 0x00)
#define GENET_TX_DMA_CTRL                       (GENET_TX_BASE + 0x1040 + 0x04)
#define GENET_TX_DMA_CTRL_RBUF_EN(qid)          __BIT((qid) + 1)
#define GENET_TX_DMA_CTRL_EN                    __BIT(0)
#define GENET_TX_SCB_BURST_SIZE                 (GENET_TX_BASE + 0x1040 + 0x0c)


#endif // _GENET_H_