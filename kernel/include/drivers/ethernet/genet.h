#ifndef _GENET_H_
#define _GENET_H_

#include "../../stdlib.h"
#include "bcm54213pe.h"
#include "../io.h"
#include "netstruct.h"


int phy_init(void)
{
    int reg;
    u32 cmd;

    // Config clock

    // Handle PHY internal RX delay
    cmd = 
    (MDIO_RD) |
    (GENET_MDIO_OFFSET << MDIO_PMD_SHIFT) |
    (MII_BCM54XX_AUX_CTL << MDIO_REG_SHIFT) |
    (0xffff & (MII_BCM54XX_AUXCTL_SHDWSEL_MASK | ((MII_BCM54XX_AUXCTL_SHDWSEL_MISC) << MII_BCM54XX_AUXCTL_SHDWSEL_READ_SHIFT))) |
    (MDIO_START_BUSY);
    chip_write(cmd, GENET_BASE_OFFSET);
    reg = chip_read(GENET_BASE_OFFSET) & 0xffff;

    reg |= MII_BCM54XX_AUXCTL_MISC_WREN;

    cmd = 
    (MDIO_WR) |
    (GENET_MDIO_OFFSET << MDIO_PMD_SHIFT) |
    (MII_BCM54XX_AUX_CTL << MDIO_REG_SHIFT) |
    (0xffff & (MII_BCM54XX_AUXCTL_SHDWSEL_MISC | reg)) |
    (MDIO_START_BUSY);
    chip_write(cmd, GENET_BASE_OFFSET);

    // Handle PHY internal TX delay
    cmd = 
    (MDIO_RD) |
    (GENET_MDIO_OFFSET << MDIO_PMD_SHIFT) |
    (MII_BCM54XX_SHD << MDIO_REG_SHIFT) |
    (0xffff & ((0x3 & 0x1f) << 10)) |
    (MDIO_START_BUSY);
    chip_write(cmd, GENET_BASE_OFFSET);
    reg = chip_read(GENET_BASE_OFFSET) & 0xffff;

    cmd = 
    (MDIO_WR) |
    (GENET_MDIO_OFFSET << MDIO_PMD_SHIFT) |
    (MII_BCM54XX_SHD << MDIO_REG_SHIFT) |
    (0xffff & (0x3 | reg)) |
    (MDIO_START_BUSY);
    chip_write(cmd, GENET_BASE_OFFSET);

    // Acknowledge interrupts and set for wake-up'
    cmd = 
    (MDIO_RD) |
    (GENET_MDIO_OFFSET << MDIO_PMD_SHIFT) |
    (MII_BCM54XX_EXP_SEL << MDIO_REG_SHIFT) |
    (0xffff & 0x0e94) |
    (MDIO_START_BUSY);
    chip_write(cmd, GENET_BASE_OFFSET);
    reg = chip_read(GENET_BASE_OFFSET) & 0xffff;

    return reg;
}

int mii_init(void)
{
    
}

int mac_init(void)
{

}

#endif // _GENET_H_