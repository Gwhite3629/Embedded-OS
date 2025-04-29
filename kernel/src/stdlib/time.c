#include <stdlib.h>
#include <drivers/io.h>
#include <drivers/platform.h>

int tick_counter = 0;

uint64_t timer_freq;

int timer_init(void)
{
    uint32_t old;

    old = chip_read(TIMER_CONTROL);
    old &= ~(TIMER_CONTROL_ENABLE | TIMER_CONTROL_INT_ENABLE);

    /* First we scale this down to 1MHz using the pre-divider */
	/* We want to /250.  The pre-divider adds one, so 249 = 0xf9 */
	chip_write(0xf9, TIMER_PREDIVIDER);

    /* We enable the /256 prescalar */
	/* So final frequency = 1MHz/256/3904 = ~1 Hz */
	chip_write(3904, TIMER_LOAD);

    /* Enable the timer in 32-bit mode, enable interrupts */
	/* And pre-scale the clock down by 256 */
	chip_write(TIMER_CONTROL_32BIT |
		TIMER_CONTROL_ENABLE |
		TIMER_CONTROL_INT_ENABLE |
		TIMER_CONTROL_PRESCALE_256,
        TIMER_CONTROL);

    // For RPI4B
    chip_write(IRQ_ENABLE_BASIC_IRQ_ARM_TIMER, IRQ0_SET_EN_2);

    return 0;
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
    //printk("WAIT: %x\n", n);
    
    // Get counter frequency
    asm volatile (
        "mrs %[result], cntfrq_el0"
        : [result] "=r"(f)
    );
    // Get current counter
    asm volatile (
        "mrs %[result], cntpct_el0"
        : [result] "=r"(t)
    );
    unsigned long i = ((f/1000)*n)/1000;
    //printk("WAIT: %x, %x, %x\n", f, t, i);

    do {
        asm volatile (
            "mrs %[result], cntpct_el0"
            : [result] "=r"(r)
        );
    } while ((r - t) < i);
    //printk("DONE WAIT\n");
}

void __attribute__((noinline)) sys_timer_init(void)
{
    asm volatile(
        "mrs x1, CNTFRQ_EL0\n"
        "msr CNTP_TVAL_EL0, x1\n"
        "mov x0, 1\n"
        "msr CNTP_CTL_EL0, x0\n"
        :::"x0","x1");
    asm volatile(
        "mrs %[result], CNTFRQ_EL0\n"
        : [result] "=r"(timer_freq));
    mmio_write(SYS_TIMER_BASE, 0xF);
}

uint64_t __attribute__((noinline)) sys_timer_read(void)
{
    uint64_t timer_val = 0;
    asm volatile(
        "mrs %[result], CNTPCT_EL0\n"
        : [result] "=r"(timer_val));
    return timer_val;
}
