#ifndef _TYPES_H_
#define _TYPES_H_

#define BITS_PER_LONG 64

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef uint64_t __u64;
typedef uint32_t __u32;
typedef uint16_t __u16;
typedef uint8_t __u8;

typedef __u64 u64;
typedef __u32 u32;
typedef __u16 u16;
typedef __u8 u8;

typedef __u32 __le32;
typedef __u16 __le16;

typedef uint32_t size_t;

typedef long int int64_t;
typedef int int32_t;
typedef signed short int16_t;
//typedef char int8_t;

typedef uint64_t uintptr_t;

typedef int bool;

#define true 1
#define false 0

typedef struct DLL {
    struct DLL *next, *prev;
} DLL;

typedef struct rb_node {
    unsigned long __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} rb_node;

typedef struct rb_root {
    struct rb_node *rb_node;
} rb_root;

struct chr_dat {
    int x0;
    int x1;
    int x;
    int y;
    unsigned char attr;
};

#define RB_ROOT (rb_root) {NULL, }

typedef int (*__compar_fn_t) (const void *, const void *);

typedef int (*__compar_d_fn_t) (const void *, const void *, void *);

#define CHAR_BIT 8

typedef struct hlist_node {
    struct hlist_node *next, **pprev;
} hlist_node;

typedef struct hlist_head {
    struct hlist_node *first;
} hlist_head;

#define __iomem __attribute__((noderef, address_space(2)))

#define __force __attribute__((force))

#define __pure __attribute__((pure))

#define __const __attribute__((const))

#define __bitwise __attribute__((bitwise))

#define __IRQ__ __attribute__((interrupt("IRQ")))
#define __UNDEF__ __attribute__((interrupt("UNDEF")))
#define __ABORT__ __attribute__((interrupt("ABORT")))
#define __FIQ__ __attribute__((interrupt("FIQ")))
#define __SVC__ __attribute__((interrupt("SVC")))
#define __SWI__ __attribute__((interrupt("SWI")))

#include "compact.h"

#endif // _TYPES_H_