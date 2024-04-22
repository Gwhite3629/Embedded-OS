#ifndef _DELAY_H_
#define _DELAY_H_

static inline void delay(int32_t count) {
	asm volatile("__delay_%=: subs %[count], %[count], #1; "
			"bne __delay_%=\n"
		: [count]"+r"(count) /* outputs */
		: /* inputs */
		: "cc" /* clobbers */);
}

#endif // _DELAY_H_