#ifndef _TYPES_H_
#define _TYPES_H_

#define size_t uint32_t

#define uint32_t unsigned int

#define int32_t int

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

typedef struct rb_root_cached {
    rb_root rb_root;
    rb_node *rb_leftmost;
} rb_root_cached;

#define RB_ROOT (rb_root) {NULL, }
#define RB_ROOT_CACHED (rb_root_cached) { {NULL, }, NULL}

#endif // _TYPES_H_