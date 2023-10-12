#ifndef _TYPES_H_
#define _TYPES_H_

#define size_t uint32_t

#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t unsigned char

#define int32_t int

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


#endif // _TYPES_H_