#ifndef _IO_H_
#define _IO_H_

#include "../stdlib/types.h"
#include "../stdlib/hardware_rpi4b.h"
#include "../stdlib/interrupts.h"

#define __raw_writel __raw_writel
static inline void __raw_writel(u32 val, volatile void __iomem *addr)
{
	asm volatile("str %1, %0"
		     : "+Qo" (*(volatile u32 *)addr)
		     : "r" (val));
}

#define __raw_readl __raw_readl
static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	u32 val;
	asm volatile("ldr %1, %0"
		     : "+Qo" (*(volatile u32 *)addr),
		       "=r" (val));
	return val;
}

#define __cpu_to_le32(x) (( __le32)(__u32)(x))
#define __le32_to_cpu(x) (( __u32)(__le32)(x))

#define writel_relaxed(v,c)	__raw_writel(( u32) __cpu_to_le32(v),c)

#define readl_relaxed(c) ({ u32 __r = __le32_to_cpu(( __le32) \
					__raw_readl(c)); __r; })

#define __iormb()		do { } while (0)
#define __iowmb()		do { } while (0)

//#define writel(v,c)		({ __iowmb(); writel_relaxed(v,c); })

//#define readl(c)		({ u32 __v = readl_relaxed(c); __iormb(); __v; })

static inline void writel(uint32_t data, uint64_t *address)
{
	/*
	asm volatile("str %[data], [%[address]]" :
		: [address]"r"(address), [data]"r"(data));
	*/
	*(volatile uint32_t *)address = data;
}

static inline uint32_t readl(uint64_t *address)
{
	/*
	uint32_t data;
	asm volatile("ldr %[data], [%[address]]" :
		[data]"=r"(data) : [address]"r"(address));
	return data;
	*/
	return *(volatile uint32_t *)address;
}

static inline void chip_write(unsigned int data, long address)
{
	memory_barrier();
    instruction_barrier();
    *(volatile unsigned int *)address = data;
	asm volatile("dmb sy");
}

static inline uint32_t chip_read(long address)
{
	uint32_t ret = *((volatile unsigned int *)(address));
	memory_barrier();
    instruction_barrier();
	return ret;
}


#endif // _IO_H_
