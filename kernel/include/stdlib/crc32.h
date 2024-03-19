#ifndef _CRC32_H_
#define _CRC32_H_

#include "types.h"
#include "bitrev.h"

u32 __pure crc32_le(u32 crc, unsigned char const *p, size_t len);
u32 __pure crc32_be(u32 crc, unsigned char const *p, size_t len);

u32 __const crc32_le_shift(u32 crc, size_t len);

static inline u32 crc32_le_combine(u32 crc1, u32 crc2, size_t len2)
{
    return crc32_le_shift(crc1, len2) ^ crc2;
}

u32 __pure __crc32c_le(u32 crc, unsigned char const *p, size_t len);

u32 __const __crc32c_le_shift(u32 crc, size_t len);

static inline u32 __crc32c_le_combine(u32 crc1, u32 crc2, size_t len2)
{
	return __crc32c_le_shift(crc1, len2) ^ crc2;
}

#define crc32(seed, data, length)  crc32_le(seed, (unsigned char const *)(data), length)

#define ether_crc(length, data)    bitrev32(crc32_le(~0, data, length))
#define ether_crc_le(length, data) crc32_le(~0, data, length)

#endif // _CRC32_H_