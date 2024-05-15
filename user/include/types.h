#ifndef _TYPES_H_
#define _TYPES_H_

#define BITS_PER_LONG 64

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef int int32_t;

typedef int bool;

typedef uint32_t size_t;

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

typedef struct hlist_node {
    struct hlist_node *next, **pprev;
} hlist_node;

typedef struct hlist_head {
    struct hlist_node *first;
} hlist_head;

typedef int (*__compar_fn_t) (const void *, const void *);

typedef int (*__compar_d_fn_t) (const void *, const void *, void *);

#define CHAR_BIT 8

#endif // _TYPES_H_