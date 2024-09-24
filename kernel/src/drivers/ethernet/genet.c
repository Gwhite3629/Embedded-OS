#include <drivers/ethernet/genet.h>
#include <drivers/ethernet/netstruct.h>

#define	MII_BUSY_RETRY      1000

/*-
 * Copyright (c) 2020 Jared McNeill <jmcneill@invisible.ca>
 * Copyright (c) 2020 Mark Kettenis <kettenis@openbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// Modified BCMGenet driver taken from openBSD

int genet_mii_read(struct eth_device *dev, int reg)
{
    int retry;

    mmio_write(GENET_MDIO_OFFSET,
        GENET_MDIO_READ | GENET_MDIO_START_BUSY |
        __SHIFTIN(dev->phy->phy_id, GENET_MDIO_PMD) |
        __SHIFTIN(reg, GENET_MDIO_REG));

    for (retry = MII_BUSY_RETRY; retry > 0; retry--) {
        if ((mmio_read(GENET_MDIO_OFFSET) & GENET_MDIO_START_BUSY) == 0)
            return mmio_read(GENET_MDIO_OFFSET) & 0xffff;
        udelay(10);
    }

    printk("PHY READ TIMEOUT\n");
    return 0;
}

void genet_mii_write(struvt eth_device *dev, int reg, int val)
{
    int retry;

    mmio_write(GENET_MDIO_OFFSET,
        val | GENET_MDIO_WRITE | GENET_MDIO_START_BUSY |
        __SHIFTIN(phy, GENET_MDIO_PMD) |
        __SHIFTIN(reg, GENET_MDIO_REG));
    for (retry = MII_BUSY_RETRY; retry > 0; retry--) {
        if ((mmio_read(GENET_MDIO_OFFSET) & GENET_MDIO_START_BUSY) == 0)
            return;
        udelay(10);
    }

    printf("PHY WRITE TIMEOUT\n");
    return;
}

void genet_disable_dma(struct eth_device *dev)
{
    uint32_t val;

    // Disable Receiver
    val = mmio_read(GENET_BASE_OFFSET + GENET_UMAC_CMD);
    val &= ~GENET_UMAC_CMD_RXEN;
    mmio_write(GENET_BASE_OFFSET + GENET_UMAC_CMD, val);

    // Stop DMA RX
    val = mmio_read(GENET_BASE_OFFSET + GENET_RX_DMA_CTRL);
    val &= ~GENET_RX_DMA_CTRL_EN;
    val &= ~GENET_RX_DMA_CTRL_RBUF_EN(GENET_DMA_DEFAULT_QUEUE);
    mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_CTRL, val);

    // Stop DMA TX
    val = mmio_read(GENET_BASE_OFFSET + GENET_TX_DMA_CTRL);
    val &= ~GENET_TX_DMA_CTRL_EN;
    val &= ~GENET_TX_DMA_CTRL_RBUF_EN(GENET_DMA_DEFAULT_QUEUE);
    mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_CTRL, val);

    // Flush TX FIFO
    mmio_write(GENET_BASE_OFFSET + GENET_UMAC_TX_FLUSH, 1);
    udelay(10);
    mmio_write(GENET_BASE_OFFSET + GENET_UMAC_TX_FLUSH, 0);

    // Disable Transmitter
    val = mmio_read(GENET_BASE_OFFSET + GENET_UMAC_CMD);
    val &= ~GENET_UMAC_CMD_TXEN;
    mmio_write(GENET_BASE_OFFSET + GENET_UMAC_CMD, val);
}

void genet_enable_intr(struct genet_softc *sc)
{
	mmio_write(GENET_BASE_OFFSET + GENET_INTRL2_CPU_CLEAR_MASK,
	    GENET_IRQ_TXDMA_DONE | GENET_IRQ_RXDMA_DONE);
}

void genet_disable_intr(struct genet_softc *sc)
{
	/* Disable interrupts */
	mmio_write(GENET_BASE_OFFSET + GENET_INTRL2_CPU_SET_MASK, 0xffffffff);
	mmio_write(GENET_BASE_OFFSET + GENET_INTRL2_CPU_CLEAR, 0xffffffff);
}

int genet_reset(struct eth_device *dev)
{
    uint32_t val;

    genet_disable_dma(dev);

    val = mmio_read(GENET_BASE_OFFSET + GENET_SYS_RBUF_FLUSH_CTRL);
	val |= GENET_SYS_RBUF_FLUSH_RESET;
	mmio_write(GENET_BASE_OFFSET + GENET_SYS_RBUF_FLUSH_CTRL, val);
	delay(10);

	val &= ~GENET_SYS_RBUF_FLUSH_RESET;
	mmio_write(GENET_BASE_OFFSET + GENET_SYS_RBUF_FLUSH_CTRL, val);
	delay(10);

	mmio_write(GENET_BASE_OFFSET + GENET_SYS_RBUF_FLUSH_CTRL, 0);
	delay(10);

	mmio_write(GENET_BASE_OFFSET + GENET_UMAC_CMD, 0);
	mmio_write(GENET_BASE_OFFSET + GENET_UMAC_CMD,
	    GENET_UMAC_CMD_LCL_LOOP_EN | GENET_UMAC_CMD_SW_RESET);
	delay(10);
	mmio_write(GENET_BASE_OFFSET + GENET_UMAC_CMD, 0);

	mmio_write(GENET_BASE_OFFSET + GENET_UMAC_MIB_CTRL, GENET_UMAC_MIB_RESET_RUNT |
	    GENET_UMAC_MIB_RESET_RX | GENET_UMAC_MIB_RESET_TX);
	mmio_write(GENET_BASE_OFFSET + GENET_UMAC_MIB_CTRL, 0);

	mmio_write(GENET_BASE_OFFSET + GENET_UMAC_MAX_FRAME_LEN, 1536);

	val = mmio_read(GENET_BASE_OFFSET + GENET_RBUF_CTRL);
	val |= GENET_RBUF_ALIGN_2B;
	mmio_write(GENET_BASE_OFFSET + GENET_RBUF_CTRL, val);

	mmio_write(GENET_BASE_OFFSET + GENET_RBUF_TBUF_SIZE_CTRL, 1);

	return 0;
}

/*
void genet_update_link(struct eth_device *dev)
{
	struct mii_data *mii = &sc->sc_mii;
	uint32_t val;
	u_int speed;

	if (IFM_SUBTYPE(mii->mii_media_active) == IFM_1000_T ||
	    IFM_SUBTYPE(mii->mii_media_active) == IFM_1000_SX)
		speed = GENET_UMAC_CMD_SPEED_1000;
	else if (IFM_SUBTYPE(mii->mii_media_active) == IFM_100_TX)
		speed = GENET_UMAC_CMD_SPEED_100;
	else
		speed = GENET_UMAC_CMD_SPEED_10;

	val = RD4(sc, GENET_EXT_RGMII_OOB_CTRL);
	val &= ~GENET_EXT_RGMII_OOB_OOB_DISABLE;
	val |= GENET_EXT_RGMII_OOB_RGMII_LINK;
	val |= GENET_EXT_RGMII_OOB_RGMII_MODE_EN;
	if (sc->sc_phy_mode == GENET_PHY_MODE_RGMII)
		val |= GENET_EXT_RGMII_OOB_ID_MODE_DISABLE;
	else
		val &= ~GENET_EXT_RGMII_OOB_ID_MODE_DISABLE;
	WR4(sc, GENET_EXT_RGMII_OOB_CTRL, val);

	val = RD4(sc, GENET_UMAC_CMD);
	val &= ~GENET_UMAC_CMD_SPEED;
	val |= __SHIFTIN(speed, GENET_UMAC_CMD_SPEED);
	WR4(sc, GENET_UMAC_CMD, val);
}
*/

void genet_init_rings(struct eth_device *sc, int qid)
{
	uint32_t val;

	/* TX ring */

	sc->sc_tx.next = 0;
	sc->sc_tx.queued = 0;
	sc->sc_tx.cidx = sc->sc_tx.pidx = 0;

	mmio_write(GENET_BASE_OFFSET + GENET_TX_SCB_BURST_SIZE, 0x08);

	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_READ_PTR_LO(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_READ_PTR_HI(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_CONS_INDEX(qid), sc->sc_tx.cidx);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_PROD_INDEX(qid), sc->sc_tx.pidx);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_RING_BUF_SIZE(qid),
	    __SHIFTIN(TX_DESC_COUNT, GENET_TX_DMA_RING_BUF_SIZE_DESC_COUNT) |
	    __SHIFTIN(MCLBYTES, GENET_TX_DMA_RING_BUF_SIZE_BUF_LENGTH));
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_START_ADDR_LO(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_START_ADDR_HI(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_END_ADDR_LO(qid),
	    TX_DESC_COUNT * GENET_DMA_DESC_SIZE / 4 - 1);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_END_ADDR_HI(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_MBUF_DONE_THRES(qid), 1);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_FLOW_PERIOD(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_WRITE_PTR_LO(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_WRITE_PTR_HI(qid), 0);

	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_RING_CFG, __BIT(qid));	/* enable */

	/* Enable transmit DMA */
	val = mmio_read(GENET_BASE_OFFSET + GENET_TX_DMA_CTRL);
	val |= GENET_TX_DMA_CTRL_EN;
	val |= GENET_TX_DMA_CTRL_RBUF_EN(qid);
	mmio_write(GENET_BASE_OFFSET + GENET_TX_DMA_CTRL, val);

	/* RX ring */

	sc->sc_rx.next = 0;
	sc->sc_rx.cidx = 0;
	sc->sc_rx.pidx = RX_DESC_COUNT;

	mmio_write(GENET_BASE_OFFSET + GENET_RX_SCB_BURST_SIZE, 0x08);

	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_WRITE_PTR_LO(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_WRITE_PTR_HI(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_PROD_INDEX(qid), sc->sc_rx.pidx);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_CONS_INDEX(qid), sc->sc_rx.cidx);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_RING_BUF_SIZE(qid),
	    __SHIFTIN(RX_DESC_COUNT, GENET_RX_DMA_RING_BUF_SIZE_DESC_COUNT) |
	    __SHIFTIN(MCLBYTES, GENET_RX_DMA_RING_BUF_SIZE_BUF_LENGTH));
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_START_ADDR_LO(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_START_ADDR_HI(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_END_ADDR_LO(qid),
	    RX_DESC_COUNT * GENET_DMA_DESC_SIZE / 4 - 1);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_END_ADDR_HI(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_XON_XOFF_THRES(qid),
	    __SHIFTIN(5, GENET_RX_DMA_XON_XOFF_THRES_LO) |
	    __SHIFTIN(RX_DESC_COUNT >> 4, GENET_RX_DMA_XON_XOFF_THRES_HI));
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_READ_PTR_LO(qid), 0);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_READ_PTR_HI(qid), 0);

	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_RING_CFG, __BIT(qid));	/* enable */

/*
	if_rxr_init(&sc->sc_rx_ring, 2, RX_DESC_COUNT);
	genet_fill_rx_ring(sc, qid);
*/

	/* Enable receive DMA */
	val = mmio_read(GENET_BASE_OFFSET + GENET_RX_DMA_CTRL);
	val |= GENET_RX_DMA_CTRL_EN;
	val |= GENET_RX_DMA_CTRL_RBUF_EN(qid);
	mmio_write(GENET_BASE_OFFSET + GENET_RX_DMA_CTRL, val);
}

int genet_init(struct eth_device *sc)
{
	struct ifnet *ifp = &sc->sc_ac.ac_if;
	struct mii_data *mii = &sc->sc_mii;
	uint32_t val;
	uint8_t *enaddr = LLADDR(ifp->if_sadl);

	if (ifp->if_flags & IFF_RUNNING)
		return 0;

	if (sc->sc_phy_mode == GENET_PHY_MODE_RGMII ||
	    sc->sc_phy_mode == GENET_PHY_MODE_RGMII_ID ||
	    sc->sc_phy_mode == GENET_PHY_MODE_RGMII_RXID ||
	    sc->sc_phy_mode == GENET_PHY_MODE_RGMII_TXID)
		WR4(sc, GENET_SYS_PORT_CTRL,
		    GENET_SYS_PORT_MODE_EXT_GPHY);

	/* Write hardware address */
	val = enaddr[3] | (enaddr[2] << 8) | (enaddr[1] << 16) |
	    (enaddr[0] << 24);
	WR4(sc, GENET_UMAC_MAC0, val);
	val = enaddr[5] | (enaddr[4] << 8);
	WR4(sc, GENET_UMAC_MAC1, val);

	/* Setup RX filter */
	genet_setup_rxfilter(sc);

	/* Setup TX/RX rings */
	genet_init_rings(sc, GENET_DMA_DEFAULT_QUEUE);

	/* Enable transmitter and receiver */
	val = RD4(sc, GENET_UMAC_CMD);
	val |= GENET_UMAC_CMD_TXEN;
	val |= GENET_UMAC_CMD_RXEN;
	WR4(sc, GENET_UMAC_CMD, val);

	/* Enable interrupts */
	genet_enable_intr(sc);

	ifp->if_flags |= IFF_RUNNING;
	ifq_clr_oactive(&ifp->if_snd);

	mii_mediachg(mii);
	timeout_add_sec(&sc->sc_stat_ch, 1);

	return 0;
}