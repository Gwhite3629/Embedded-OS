#ifndef _STRACE_H_
#define _STRACE_H_

#include "../stdlib/string.h"
#include "../stdlib/printk.h"

struct __attribute__((__packed__, __aligned__(16))) trace_element {
    char func_name[32];
    char pretty_func_name[64];
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
};

extern struct trace_element trace_stack[100];
extern uint32_t __depth;
extern struct chr_dat __info;
extern uint32_t __new;

#define push_trace(__pretty, __f, __x0, __x1, __x2, __x3) \
    do { \
        strncpy(trace_stack[__depth].func_name, __f, strlen(__f)); \
        strncpy(trace_stack[__depth].pretty_func_name, __pretty, strlen(__pretty)); \
        trace_stack[__depth].x0 = (uint64_t)__x0; \
        trace_stack[__depth].x1 = (uint64_t)__x1; \
        trace_stack[__depth].x2 = (uint64_t)__x2; \
        trace_stack[__depth].x3 = (uint64_t)__x3; \
        print_screen(&__info, "%s(%x%x, %x%x, %x%x, %x%x)\n",\
        trace_stack[__depth].func_name,\
        (trace_stack[__depth].x0 & 0xffffffff) << 32,\
         trace_stack[__depth].x0 & 0xffffffff,\
        (trace_stack[__depth].x1 & 0xffffffff) << 32,\
         trace_stack[__depth].x1 & 0xffffffff,\
        (trace_stack[__depth].x2 & 0xffffffff) << 32,\
         trace_stack[__depth].x2 & 0xffffffff,\
        (trace_stack[__depth].x3 & 0xffffffff) << 32,\
         trace_stack[__depth].x3 & 0xffffffff);\
        __depth++; \
        __new = 0; \
    } while(0)


#define pop_trace() \
    do { \
        __depth--; \
        memset(trace_stack[__depth].func_name, 0, 32); \
        memset(trace_stack[__depth].pretty_func_name, 0, 64); \
        trace_stack[__depth].x0 = 0; \
        trace_stack[__depth].x1 = 0; \
        trace_stack[__depth].x2 = 0; \
        trace_stack[__depth].x3 = 0; \
        __new = 1; \
    } while(0)


//#define push_trace(__pretty,__f,__x0,__x1,__x2,__x3)

//#define pop_trace()

void dump_trace(uint32_t n);

#endif // _STRACE_H_
