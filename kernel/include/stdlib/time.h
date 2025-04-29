#ifndef _TIME_H_
#define _TIME_H_

#include "types.h"

extern uint64_t timer_freq;

int timer_init(void);
void wait_msec(unsigned int n);
void wait_cycles(unsigned int n);

void sys_timer_init(void);

uint64_t sys_timer_read(void);

#define udelay(n) wait_msec(n)

#endif // _TIME_H_
