#ifndef _PHY_H_
#define _PHY_H_

#include "bcm54213pe.h"
#include "netstruct.h"
#include "../io.h"
#include "mii.h"
#include "eth.h"

#include <memory/memory.h>

#define MDIO_CMD		0x00
#define  MDIO_START_BUSY	(1 << 29)
#define  MDIO_READ_FAIL		(1 << 28)
#define  MDIO_RD		(2 << 26)
#define  MDIO_WR		(1 << 26)
#define  MDIO_PMD_SHIFT		21
#define  MDIO_PMD_MASK		0x1F
#define  MDIO_REG_SHIFT		16
#define  MDIO_REG_MASK		0x1F

#define MDIO_CFG		0x04
#define  MDIO_C22		(1 << 0)
#define  MDIO_C45		0
#define  MDIO_CLK_DIV_SHIFT	4
#define  MDIO_CLK_DIV_MASK	0x3F
#define  MDIO_SUPP_PREAMBLE	(1 << 12)


struct bcm_phy_hw_stat {
	const char *string;
	int devad;
	u16 reg;
	u8 shift;
	u8 bits;
};

static const struct bcm_phy_hw_stat bcm_phy_hw_stats[] = {
	{ "phy_receive_errors", -1, MII_BRCM_CORE_BASE12, 0, 16 },
	{ "phy_serdes_ber_errors", -1, MII_BRCM_CORE_BASE13, 8, 8 },
	{ "phy_false_carrier_sense_errors", -1, MII_BRCM_CORE_BASE13, 0, 8 },
	{ "phy_local_rcvr_nok", -1, MII_BRCM_CORE_BASE14, 8, 8 },
	{ "phy_remote_rcv_nok", -1, MII_BRCM_CORE_BASE14, 0, 8 },
	{ "phy_lpi_count", MDIO_MMD_AN, BRCM_CL45VEN_EEE_LPI_CNT, 0, 16 },
};

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static inline int phy_read(struct phy_device *phydev, u32 regnum)
{
    // mdiobus_read(phydev->mdio.bus, phydev->mdio.addr, regnum);
    // |--> bus->read(bus, addr, regnum);
    //      |--> unimac_mdio_read(bus, phy_id, reg);

    u32 cmd;
    u32 reg;

    int phy_id = phydev->mdio.addr;

    cmd = MDIO_RD | (phy_id << MDIO_PMD_SHIFT) | (regnum << MDIO_REG_SHIFT);
    chip_write(cmd, (unsigned int)phydev->base + MDIO_CMD);

    reg = chip_read((unsigned int)phydev->base + MDIO_CMD);
    reg |=  MDIO_START_BUSY;
    chip_write(reg, (unsigned int)phydev->base + MDIO_CMD);

    // Wait func here

    cmd = chip_read((unsigned int)phydev->base + MDIO_CMD);

    return cmd & 0xffff;
}

static int phy_write(struct phy_device *phydev, u32 regnum, u16 val)
{
    // mdiobus_write(phydev->mdio.bus, phydev->mdio.addr, regnum, val);
    // |--> bus->write(bus, addr, regnum, val);
    //      |--> unimac_mdio_write(bus, phy_id, reg, val);

    u32 cmd;
    u32 reg;

    int phy_id = phydev->mdio.addr;

    cmd = MDIO_WR | (phy_id << MDIO_PMD_SHIFT) | (regnum << MDIO_REG_SHIFT) | (0xffff & val);
    chip_write(cmd, (unsigned int)phydev->base + MDIO_CMD);

    reg = chip_read((unsigned int)phydev->base + MDIO_CMD);
    reg |=  MDIO_START_BUSY;
    chip_write(reg, (unsigned int)phydev->base + MDIO_CMD);

    // wait func here
    return 0;
}

static int phy_shadow_read(struct phy_device *phydev, u16 shadow)
{
    int ret;
    phy_write(phydev, MII_BCM54XX_SHD, (shadow & 0x1f) << 10);
    ret = phy_read(phydev, MII_BCM54XX_SHD);
    return ((ret & 0x3ff) << 0);
}

static int phy_shadow_write(struct phy_device *phydev, u16 shadow, u16 val)
{
    int ret;
    ret = phy_write(phydev, MII_BCM54XX_SHD,
        MII_BCM54XX_SHD_WRITE |
        ((shadow & 0x1f) << 10) |
        ((val & 0x3ff) << 0));
    return ret;
}

static int phy_auxctl_read(struct phy_device *phydev, u16 regnum)
{
    int ret;
    phy_write(phydev, MII_BCM54XX_AUX_CTL, MII_BCM54XX_AUXCTL_SHDWSEL_MASK |
        regnum << MII_BCM54XX_AUXCTL_SHDWSEL_READ_SHIFT);
    ret = phy_read(phydev, MII_BCM54XX_AUX_CTL);
    return ret;
}

static int phy_auxctl_write(struct phy_device *phydev, u16 regnum, u16 val)
{
    int ret;
    ret = phy_write(phydev, MII_BCM54XX_AUX_CTL, regnum | val);
    return ret;
}

static int phy_exp_read(struct phy_device *phydev, u16 reg)
{
    int val;
    val = phy_write(phydev, MII_BCM54XX_EXP_SEL, reg);
    if (val < 0) {
        return val;
    }

    val = phy_read(phydev, MII_BCM54XX_EXP_DATA);

    phy_write(phydev, MII_BCM54XX_EXP_SEL, 0);

    return val;
}

static int phy_exp_write(struct phy_device *phydev, u16 regnum, u16 val)
{
    int rc;

    rc = phy_write(phydev, MII_BCM54XX_EXP_SEL, regnum);
    if (rc < 0)
        return rc;

    return phy_write(phydev, MII_BCM54XX_EXP_DATA, val);
}

static inline int phy_clear_bits(struct phy_device *phydev, u32 regnum, u16 val)
{
    // phy_modify(phydev, regnum, val, 0);
    // |--> __phy_modify(phydev, regnum, mask, set);
    //      |--> __phy_modify_changed(phydev, regnum, mask, set);
    //           |--> __mdiobus_modify_changed(phydev->mdio.bus, phydev->mdio.addr,
    //                  regnum, mask, set)
    //                |--> ret = __mdio_bus_read(bus, addr, regnum);
    //                     new = (ret & ~mask) | set;
    //                     if (new == ret) return 0;
    //                     ret = __mdiobus_write(bus, addr, regnum, new);

    // unimac_mdio_write(bus, phy_id, reg, val)
    // |--> phy_write()
    // unimac_mdio_read(bus, phy_id, reg)
    // |--> phy_read()

    int ret, newv = 0;

    ret = phy_read(phydev, regnum);
    
    newv = (ret & ~val) | 0;

    if (newv == ret) {
        return 0;
    }

    ret = phy_write(phydev, regnum, newv);

    return ret;
}

int (*soft_reset)(struct phy_device *phydev);

int bcm54xx_config_init(struct phy_device *phydev)
{
    int reg, val, err, rc;

    // Read from extended control register via MDIO
    reg = phy_read(phydev, MII_BCM54XX_ECR);
    if (reg < 0) {
        return reg;
    }

    // Mask global interrupts
    reg |= MII_BCM54XX_ECR_IM;
    err = phy_write(phydev, MII_BCM54XX_ECR, reg);
    if (err < 0) {
        return err;
    }

    // Unmask duplex speed and link interrupts
    reg = ~(MII_BCM54XX_INT_DUPLEX |
		MII_BCM54XX_INT_SPEED |
		MII_BCM54XX_INT_LINK);
	err = phy_write(phydev, MII_BCM54XX_IMR, reg);
	if (err < 0)
		return err;

    // Adjust receive reference clock
    // Untested for our PHY so not implemented
    
    // Config clock delay

    // Handle internal RX clock delay
    val = phy_auxctl_read(phydev, MII_BCM54XX_AUXCTL_SHDWSEL_MISC);
    val |= MII_BCM54XX_AUXCTL_MISC_WREN;
    val &= ~MII_BCM54XX_AUXCTL_SHDWSEL_MISC_RGMII_SKEW_EN;
    rc = phy_auxctl_write(phydev, MII_BCM54XX_AUXCTL_SHDWSEL_MISC, val);
    if (rc < 0) {
        return rc;
    }

    // Handle internal TX clock delay
    val = phy_shadow_read(phydev, BCM54810_SHD_CLK_CTL);
    val &= ~BCM54810_SHD_CLK_CTL_GTXCLK_EN;
    rc = phy_shadow_write(phydev, BCM54810_SHD_CLK_CTL, val);
    if (rc < 0) {
        return rc;
    }

    if (phydev->flags & PHY_BRCM_EN_MASTER_MODE) {
        val = phy_read(phydev, MII_CTRL1000);
        val |= CTL1000_AS_MASTER | CTL1000_ENABLE_MASTER;
        phy_write(phydev, MII_CTRL1000, val);
    }

    // PHY dsp config

    // Enable SMDSP clock
    err = phy_auxctl_write(phydev,
        MII_BCM54XX_AUXCTL_SHDWSEL_AUXCTL,
        MII_BCM54XX_AUXCTL_ACTL_SMDSP_ENA | 
        MII_BCM54XX_AUXCTL_ACTL_TX_6DB);
    if (err < 0) {
        return err;
    }

    // Setup LEDs
    // Skipped for now

    // Acknowledge left over interrupts and wake-up;
    err = phy_exp_read(phydev, BCM54XX_WOL_INT_STATUS);
    if (err < 0) {
        return err;
    }

    return 0;
}

int bcm54xx_probe(struct phy_device *phydev)
{
    struct bcm54xx_phy_priv *priv;
    int ret = 0;

    new(priv, 1, bcm54xx_phy_priv);

    priv->wake_irq = E_NXIO;

    phydev->priv = priv;

    new(priv->stats, ARRAY_SIZE(bcm_phy_hw_stats), u64);

    // Need to call device wakeup function here
    return 1;
}

int bcm54xx_suspend(struct phy_device *phydev)
{
    int ret = 0;

    ret = phy_exp_read(phydev, BCM54XX_WOL_INT_STATUS);
    if (ret < 0) {
        return ret;
    }

    ret = phy_write(phydev, MII_BMCR, BMCR_PDOWN);
    if (ret < 0) {
        return ret;
    }

    return ret;
}

int bcm54xx_resume(struct phy_device *phydev)
{
    int ret = 0;

    ret = phy_clear_bits(phydev, MII_BMCR, BMCR_PDOWN);

    // Sleep for 40us

    return bcm54xx_config_init(phydev);
}

int (*config_aneg)(struct phy_device *phydev);
int (*aneg_done)(struct phy_device *phydev);
int (*read_status)(struct phy_device *phydev);
int (*config_intr)(struct phy_device *phydev);
int (*handle_interrupt)(struct phy_device *phydev);
void (*remove)(struct phy_device *phydev);

void bcm54xx_link_change_notify(struct phy_device *phydev)
{
    u16 mask =  MII_BCM54XX_EXP_EXP08_EARLY_DAC_WAKE |
                MII_BCM54XX_EXP_EXP08_FORCE_DAC_WAKE;
    int ret;

    if (phydev->state != PHY_RUNNING)
        return;

    if (!(phydev->flags & PHY_BRCM_AUTO_PWRDWN_ENABLE))
        return;

    ret = phy_exp_read(phydev, MII_BCM54XX_EXP_EXP08);
    if (ret < 0)
        return;

    if (phydev->speed == SPEED_10)
        ret |= mask;
    else
        ret &= ~mask;

    phy_exp_write(phydev, MII_BCM54XX_EXP_EXP08, ret);
}

#endif // _PHY_H_