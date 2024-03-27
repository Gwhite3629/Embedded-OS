#ifndef _BITREV_H_
#define _BITREV_H_

#include "types.h"

extern u8 const byte_rev_table[256];

static inline u8 __bitrev8(u8 byte)
{
	return byte_rev_table[byte];
}

static inline u16 __bitrev16(u16 x)
{
	return (__bitrev8(x & 0xff) << 8) | __bitrev8(x >> 8);
}

static inline u32 __bitrev32(u32 x)
{
	return (__bitrev16(x & 0xffff) << 16) | __bitrev16(x >> 16);
}

#define __constant_bitrev32(x)	\
({					\
	u32 __x = x;			\
	__x = (__x >> 16) | (__x << 16);	\
	__x = ((__x & (u32)0xFF00FF00UL) >> 8) | ((__x & (u32)0x00FF00FFUL) << 8);	\
	__x = ((__x & (u32)0xF0F0F0F0UL) >> 4) | ((__x & (u32)0x0F0F0F0FUL) << 4);	\
	__x = ((__x & (u32)0xCCCCCCCCUL) >> 2) | ((__x & (u32)0x33333333UL) << 2);	\
	__x = ((__x & (u32)0xAAAAAAAAUL) >> 1) | ((__x & (u32)0x55555555UL) << 1);	\
	__x;								\
})

#define __constant_bitrev16(x)	\
({					\
	u16 __x = x;			\
	__x = (__x >> 8) | (__x << 8);	\
	__x = ((__x & (u16)0xF0F0U) >> 4) | ((__x & (u16)0x0F0FU) << 4);	\
	__x = ((__x & (u16)0xCCCCU) >> 2) | ((__x & (u16)0x3333U) << 2);	\
	__x = ((__x & (u16)0xAAAAU) >> 1) | ((__x & (u16)0x5555U) << 1);	\
	__x;								\
})

#define __constant_bitrev8(x)	\
({					\
	u8 __x = x;			\
	__x = (__x >> 4) | (__x << 4);	\
	__x = ((__x & (u8)0xCCU) >> 2) | ((__x & (u8)0x33U) << 2);	\
	__x = ((__x & (u8)0xAAU) >> 1) | ((__x & (u8)0x55U) << 1);	\
	__x;								\
})

#define bitrev32(x) \
({			\
	u32 __x = x;	\
	__builtin_constant_p(__x) ?	\
	__constant_bitrev32(__x) :			\
	__bitrev32(__x);				\
})

#define bitrev16(x) \
({			\
	u16 __x = x;	\
	__builtin_constant_p(__x) ?	\
	__constant_bitrev16(__x) :			\
	__bitrev16(__x);				\
 })

#define bitrev8(x) \
({			\
	u8 __x = x;	\
	__builtin_constant_p(__x) ?	\
	__constant_bitrev8(__x) :			\
	__bitrev8(__x)	;			\
 })

#endif // _BITREV_H_