#include <stdlib.h>
#include <drivers/io.h>
#include <drivers/gic400.h>

uint32_t num_interrupts = 0;

/* Initialize gic400 interrupt controller as found on a Raspberry pi4 */
/* Note: this just does the bare minimum to get interrupts delivered */

uint32_t gic400_init(void* interrupt_controller_base) {

	int n;

    /* Disable the controller so we can configure it before it passes any
       interrupts to the CPU */
    chip_write(GICD_CTLR_DISABLE, (GICD_CTLR));

	/* Get the number of interrupt lines implemented */
	/* (number of registers * 32) in the GIC400 controller */
	num_interrupts=(chip_read(GICD_TYPE)&0xf)*32;
//	num_interrupts=80;

	/* disable, ack, and deactivate all */
	for (n = 0; n < num_interrupts/32; n++) {
        chip_write(0xffffffffUL, (GICD_ICENABLER0 + 4*n));
        chip_write(0xffffffffUL, (GICD_ICPENDR0   + 4*n));
        chip_write(0xffffffffUL, (GICD_ICACTIVER0 + 4*n));
	}

	/* direct all interrupts to core 0 with default priority */
	for (n = 0; n < num_interrupts/4; n++) {

		/* Priority default is 0xA0 */
        chip_write(GICD_IPRIORITYR_DEFAULT |
				GICD_IPRIORITYR_DEFAULT << 8 |
				GICD_IPRIORITYR_DEFAULT << 16 |
				GICD_IPRIORITYR_DEFAULT << 24,
                (GICD_IPRIORITYR0 + 4*n));

        chip_write(GICD_ITARGETSR_CORE0 |
				GICD_ITARGETSR_CORE0 << 8 |
				GICD_ITARGETSR_CORE0 << 16 |
				GICD_ITARGETSR_CORE0 << 24,
                (GICD_ITARGETSR0 + 4*n));
	}

	/* set all interrupts to level triggered */
	for (n = 0; n < num_interrupts/16; n++) {
        chip_write(0, (GICD_ICFGR0 + 4*n));
	}

	/* Enable PPI interrupts */
//	mmio2_write(GICD_ISENABLER0,0xffffffffUL);
    chip_write(0x80000000UL, (GICD_ISENABLER0));

	/* enable */
    chip_write(GICD_CTLR_ENABLE, (GICD_CTLR));

	/* initialize core 0 CPU interface: */
    chip_write(GICC_PMR_PRIORITY, (GICC_PMR));
    chip_write(GICC_CTLR_ENABLE, (GICC_CTLR));

	return 0;
}
