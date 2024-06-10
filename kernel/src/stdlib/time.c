#include <stdlib.h>
#include <drivers/io.h>

int tick_counter = 0;

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
struct arm_delay_ops arm_delay_ops = {
	.delay		= __timer_delay,
	.const_udelay	= __timer_const_udelay,
	.udelay		= __timer_udelay,
};

void
clocks_calc_mult_shift(u32 *mult, u32 *shift, u32 from, u32 to, u32 maxsec)
{
	u64 tmp;
	u32 sft, sftacc= 32;

	/*
	 * Calculate the shift factor which is limiting the conversion
	 * range:
	 */
	tmp = ((u64)maxsec * from) >> 32;
	while (tmp) {
		tmp >>=1;
		sftacc--;
	}

	/*
	 * Find the conversion shift/mult pair which has the best
	 * accuracy and fits the maxsec conversion range:
	 */
	for (sft = 32; sft > 0; sft--) {
		tmp = (u64) to << sft;
		tmp += from / 2;
		tmp /= from;
		if ((tmp >> sftacc) == 0)
			break;
	}
	*mult = tmp;
	*shift = sft;
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