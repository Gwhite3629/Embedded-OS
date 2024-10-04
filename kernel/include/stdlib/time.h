#ifndef _TIME_H_
#define _TIME_H_

extern int tick_counter;

int timer_init(void);
void wait_msec(unsigned int n);
void wait_cycles(unsigned int n);

#define udelay(n) wait_msec(n)

#endif // _TIME_H_
