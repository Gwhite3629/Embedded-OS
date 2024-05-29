#ifndef _BITMAP_H_
#define _BITMAP_H_
typedef unsigned int* bitmap_t;
#define MAP_SIZE(n) ((n)/8)+((n)&7)
#define set_bit(words, n) (words[(n)/32] |= (1 << ((n) & 31)))
#define clear_bit(words, n) (words[(n)/32] &= ~(1 << ((n) & 31)))
#define get_bit(words, n) (words[(n)/32] & (1 << ((n)&31)))
#endif