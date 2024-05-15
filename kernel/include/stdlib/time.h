#ifndef _TIME_H_
#define _TIME_H_

extern int tick_counter;

#define HZ 100

#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define FSEC_PER_SEC	1000000000000000LL

typedef unsigned long cycles_t;
#define get_cycles()	({ cycles_t c; read_current_timer(&c) ? 0 : c; })

#define MAX_UDELAY_MS	2
#define UDELAY_MULT	((((unsigned long)2199023) * HZ) >> 11)
#define UDELAY_SHIFT	30

int timer_init(void);

void wait_cycles(unsigned int n);
void wait_msec(unsigned int n);

struct delay_timer {
	unsigned long (*read_current_timer)(void);
	unsigned long freq;
};

extern struct arm_delay_ops {
	void (*delay)(unsigned long);
	void (*const_udelay)(unsigned long);
	void (*udelay)(unsigned long);
	unsigned long ticks_per_jiffy;
} arm_delay_ops;

#define __delay(n)		arm_delay_ops.delay(n)

extern void __bad_udelay(void);

extern void register_current_timer_delay(const struct delay_timer *timer);

#define __udelay(n)		arm_delay_ops.udelay(n)
#define __const_udelay(n)	arm_delay_ops.const_udelay(n)

#define udelay(n)							\
	(__builtin_constant_p(n) ?					\
	  ((n) > (MAX_UDELAY_MS * 1000) ? __bad_udelay() :		\
			__const_udelay((n) * UDELAY_MULT)) :		\
	  __udelay(n))

/* Loop-based definitions for assembly code. */
extern void __loop_delay(unsigned long loops);
extern void __loop_udelay(unsigned long usecs);
extern void __loop_const_udelay(unsigned long);

#endif // _TIME_H_
