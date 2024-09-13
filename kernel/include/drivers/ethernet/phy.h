#ifndef _PHY_H_
#define _PHY_H_

#include "bcm54213pe.h"
#include "netstruct.h"
#include "../io.h"
#include "mii.h"
#include "eth.h"

#include <memory/memory.h>

#define  MDIO_CMD		0x00
#define  MDIO_START_BUSY	(1 << 29)
#define  MDIO_READ_FAIL		(1 << 28)
#define  MDIO_RD		(2 << 26)
#define  MDIO_WR		(1 << 26)
#define  MDIO_PMD_SHIFT		21
#define  MDIO_PMD_MASK		0x1F
#define  MDIO_REG_SHIFT		16
#define  MDIO_REG_MASK		0x1F

#define  MDIO_CFG		0x04
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


#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


static inline int phy_read(struct phy_device *phydev, u32 regnum);
static int phy_write(struct phy_device *phydev, u32 regnum, u16 val);
static int phy_shadow_read(struct phy_device *phydev, u16 shadow);
static int phy_shadow_write(struct phy_device *phydev, u16 shadow, u16 val);
static int phy_auxctl_read(struct phy_device *phydev, u16 regnum);
static int phy_auxctl_write(struct phy_device *phydev, u16 regnum, u16 val);
static int phy_exp_read(struct phy_device *phydev, u16 reg);
static int phy_exp_write(struct phy_device *phydev, u16 regnum, u16 val);
static inline int phy_clear_bits(struct phy_device *phydev, u32 regnum, u16 val);
int bcm54xx_config_init(struct phy_device *phydev);
int bcm54xx_probe(struct phy_device *phydev);
int bcm54xx_suspend(struct phy_device *phydev);
int bcm54xx_resume(struct phy_device *phydev);
int bcm54xx_config_intr(struct phy_device *phydev);
int bcm54xx_handle_interrupt(struvt phy_device *phydev);
void bcm54xx_link_change_notify(struct phy_device *phydev);

#endif // _PHY_H_