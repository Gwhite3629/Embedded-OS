#ifndef _EXTRA_H_
#define _EXTRA_H_

#include "types.h"

#define __INT_MAX__ 2147483647
#define __UINT_MAX__ 4294967295U

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define unlikely(x)		__builtin_expect(!!(x), 0)
#define likely(x)		__builtin_expect(!!(x), 1)

#define barrier() __asm__ __volatile__("": : :"memory")

static inline void __read_once_size(const volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(uint8_t  *) res = *(volatile uint8_t  *) p; break;
	case 2: *(uint16_t *) res = *(volatile uint16_t *) p; break;
	case 4: *(uint32_t *) res = *(volatile uint32_t *) p; break;
	default:
		barrier();
		__builtin_memcpy((void *)res, (const void *)p, size);
		barrier();
	}
}

static inline void __write_once_size(volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(volatile  uint8_t *) p = *(uint8_t  *) res; break;
	case 2: *(volatile uint16_t *) p = *(uint16_t *) res; break;
	case 4: *(volatile uint32_t *) p = *(uint32_t *) res; break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

#define READ_ONCE(x)					\
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__c = { 0 } };			\
	__read_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})

#define WRITE_ONCE(x, val)				\
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__val = (val) }; 			\
	__write_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})

static inline int ilog2(unsigned long v)
{
	int l = 0;
	while ((1UL << l) < v)
		l++;
	return l;
}

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define EXPORT(x)

static inline void cpu_relax(void)
{
	asm volatile("yield" ::: "memory");
}

#endif // _EXTRA_H_