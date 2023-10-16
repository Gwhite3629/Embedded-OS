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

typedef struct DLL {
    DLL *next, *prev;
} DLL;

typedef struct rb_node {
    unsigned long __rb_parent_color;
	rb_node *rb_right;
	rb_node *rb_left;
} rb_node;

typedef struct rb_root {
    rb_node *rb_node;
} rb_root;

#define RB_ROOT (rb_root) {NULL, }

typedef int (*__compar_fn_t) (const void *, const void *);

typedef int (*__compar_d_fn_t) (const void *, const void *, void *);

#define CHAR_BIT 8

struct hlist_node {
    struct hlist_node *next, **pprev;
};

struct hlist_head {
    struct hlist_node *first;
};

#include "compact.h"

#endif // _TYPES_H_