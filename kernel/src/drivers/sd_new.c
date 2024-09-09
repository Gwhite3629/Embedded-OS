#include <drivers/sd.h>

void sd_set_power(bool state)
{
    asm volatile("dmb sy");
    mmio_write(SD_BASE + SD_VDD, state);
}

void sd_reset(void)
{
    sd_set_power(0);

    asm volatile("dmb sy");

    // Reset registers
    mmio_write(SD_BASE + SD_CMD, 0);
    mmio_write(SD_BASE + SD_ARG, 0);
    mmio_write(SD_BASE + SD_TOUT, 0xf00000);
    mmio_write(SD_BASE + SD_CDIV, 0);
    mmio_write(SD_BASE + SD_HSTS, 0x7f8);
    mmio_write(SD_BASE + SD_HCFG, 0);
    mmio_write(SD_BASE + SD_HBCT, 0);
    mmio_write(SD_BASE + SD_HBLC, 0);

    udelay(10000);

    sd_set_power(1);

    udelay(10000);

    mmio_write(SD_BASE + SD_HCFG, 1 << 10);
    mmio_write(SD_BASE + SD_CDIV, 0x7ff);

    asm volatile("dmb sy");
}

int sd_card_init(struct block_device **dev)
{
    // Check GPIO
    // Set GPIOs 1st pull non then rest pull up

    // Reset host clock
    uint32_t sd_set_base_clock_hz(0, 0xFFFFFFFF, 0xFFFFFFFF);

    sd_reset();

    

}