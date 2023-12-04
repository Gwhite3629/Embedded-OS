#ifndef _TYPES_H_
#define _TYPES_H_

#define BITS_PER_LONG 64

typedef uint32_t size_t;

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef int int32_t;

typedef int bool;

#define true 1
#define false 0

typedef int (*__compar_fn_t) (const void *, const void *);

typedef int (*__compar_d_fn_t) (const void *, const void *, void *);

#define CHAR_BIT 8

#endif // _TYPES_H_