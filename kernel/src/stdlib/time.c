#include <stdlib.h>
#include <drivers/io.h>

int tick_counter = 0;

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
    writel(IO_BASE + IRQ0_SET_EN_2, IRQ_ENABLE_BASIC_IRQ_ARM_TIMER);

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

struct arm_delay_ops arm_delay_ops = {
	.delay		= __loop_delay,
	.const_udelay	= __loop_const_udelay,
	.udelay		= __loop_udelay,
};

static const struct delay_timer *delay_timer;
static bool delay_calibrated;
static u64 delay_res;

unsigned long lpj_fine;

int read_current_timer(unsigned long *timer_val)
{
	if (!delay_timer)
		return E_NXIO;

	*timer_val = delay_timer->read_current_timer();
	return 0;
}

static inline u64 cyc_to_ns(u64 cyc, u32 mult, u32 shift)
{
	return (cyc * mult) >> shift;
}

static void __timer_delay(unsigned long cycles)
{
	cycles_t start = get_cycles();

	while ((get_cycles() - start) < cycles)
		cpu_relax();
}

static void __timer_const_udelay(unsigned long xloops)
{
	unsigned long long loops = xloops;
	loops *= arm_delay_ops.ticks_per_jiffy;
	__timer_delay(loops >> UDELAY_SHIFT);
}

static void __timer_udelay(unsigned long usecs)
{
	__timer_const_udelay(usecs * UDELAY_MULT);
}

void register_current_timer_delay(const struct delay_timer *timer)
{
	u32 new_mult, new_shift;
	u64 res;

	clocks_calc_mult_shift(&new_mult, &new_shift, timer->freq,
			       NSEC_PER_SEC, 3600);
	res = cyc_to_ns(1ULL, new_mult, new_shift);

	if (res > 1000) {
		return;
	}

	if (!delay_calibrated && (!delay_res || (res < delay_res))) {
		delay_timer			= timer;
		lpj_fine			= timer->freq / HZ;
		delay_res			= res;

		/* cpufreq may scale loops_per_jiffy, so keep a private copy */
		arm_delay_ops.ticks_per_jiffy	= lpj_fine;
		arm_delay_ops.delay		= __timer_delay;
		arm_delay_ops.const_udelay	= __timer_const_udelay;
		arm_delay_ops.udelay		= __timer_udelay;
	}
}

void __bad_udelay(void)
{
    return;
}
/*
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
*/