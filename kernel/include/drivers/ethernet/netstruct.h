#ifndef _NETSTRUCT_H_
#define _NETSTRUCT_H_

#include "../../stdlib.h."

struct mdio_device {
    int addr;
    int flags;
};

struct phy_driver {
    int (*soft_reset)(struct phy_device *phydev);
    int (*config_init)(struct phy_device *phydev);
    int (*probe)(struct phy_device *phydev);
    int (*suspend)(struct phy_device *phydev);
	int (*resume)(struct phy_device *phydev);
    int (*config_aneg)(struct phy_device *phydev);
	int (*aneg_done)(struct phy_device *phydev);
	int (*read_status)(struct phy_device *phydev);
    int (*config_intr)(struct phy_device *phydev);
	int (*handle_interrupt)(struct phy_device *phydev);
	void (*remove)(struct phy_device *phydev);
	void (*link_change_notify)(struct phy_device *dev);
};

enum phy_state {
	PHY_DOWN = 0,
	PHY_READY,
	PHY_HALTED,
	PHY_ERROR,
	PHY_UP,
	PHY_RUNNING,
	PHY_NOLINK,
	PHY_CABLETEST,
};

struct bcm54xx_phy_priv {
    u64 *stats;
    int wake_irq;
    bool wake_irq_enabled;
};

// Phy device struct
// Required functions:
//  link_change_notify
struct phy_device {
    struct mdio_device mdio;
    struct phy_driver *drv;

    u32 phy_id;

    unsigned autoneg:1;
    unsigned link:1;
    unsigned autoneg_complete:1;
    unsigned interrupts:1;
    unsigned irq_suspended:1;
    unsigned irq_rerun:1;

    int speed;
    int duplex;
    int port;
    int pause;
    int flags;

    int irq;

    enum phy_state state;

    struct bcm54xx_phy_priv *priv;

    void *base;
};

struct eth_device {
    struct phy_device *phy;

};

#endif // _NETSTRUCT_H_