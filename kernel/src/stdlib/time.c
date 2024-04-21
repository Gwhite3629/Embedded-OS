#include <stdlib.h>
#include <drivers/io.h>

int timer_init(void)
{
    uint32_t old;

    old = readl(IO_BASE + TIMER_CONTROL);
    old &= ~(TIMER_CONTROL_ENABLE | TIMER_CONTROL_INT_ENABLE);

    /* First we scale this down to 1MHz using the pre-divider */
	/* We want to /250.  The pre-divider adds one, so 249 = 0xf9 */
    writel(0xf9, IO_BASE + TIMER_PREDIVIDER);

    /* We enable the /256 prescalar */
	/* So final frequency = 1MHz/256/3904 = ~1 Hz */
    writel(3904, IO_BASE + TIMER_LOAD);

    /* Enable the timer in 32-bit mode, enable interrupts */
	/* And pre-scale the clock down by 256 */
	writel(TIMER_CONTROL_32BIT |
		TIMER_CONTROL_ENABLE |
		TIMER_CONTROL_INT_ENABLE |
		TIMER_CONTROL_PRESCALE_256,
        IO_BASE + TIMER_CONTROL);

    // For RPI4B
    bcm2835_write(IRQ_ENABLE_BASIC_IRQ_ARM_TIMER,IO_BASE + IRQ0_SET_EN_2);

}

void wait_cycles(unsigned int n)
{
    if(n) {
        while(n--) {
            asm volatile(
                "nop"
            );
        }
    }
}

void wait_msec(unsigned int n)
{
    register unsigned long f, t, r;
    
    // Get counter frequency
    asm volatile (
        "mrs %0, cntfrq_el0"
        : "=r"(f)
    );
    // Get current counter
    asm volatile (
        "mrs &0, cntpct_el0"
        : "=r"(t)
    );
    unsigned long i = ((f/1000)*n)/1000;

    do {
        asm volatile (
            "mrs %0, cntpct_el0"
            : "=r"(r)
        );
    } while (r - t < i);
}
