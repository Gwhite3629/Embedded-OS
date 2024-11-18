#include <trace/strace.h>
#include <stdlib.h>

struct trace_element trace_stack[100];
uint32_t __depth;
struct chr_dat __info;
uint32_t __new;

void dump_trace(uint32_t n)
{
    if (n) {
        draw_rect(648, 32, 1295, 1079, 0x52DFDB, 1);
    }
    struct chr_dat info = {652, 44, 652, 44, 0};
    struct chr_dat tmp = info;
    tmp.y0 -=8;
    tmp.y -=8;
    print_screen(&tmp,"STACK TRACE");
 
    for (uint32_t i = 0; i < __depth; i++) {
    //    print_screen(&info, "FUNCTION: %3d\n", i);
    //    print_screen(&info, "%s\n", trace_stack[__depth].pretty_func_name);
        print_screen(&info, "%s(%x%x, %x%x, %x%x, %x%x)\n",
         trace_stack[i].func_name,
        (trace_stack[i].x0 & 0xffffffff) << 32,
         trace_stack[i].x0 & 0xffffffff,
        (trace_stack[i].x1 & 0xffffffff) << 32,
         trace_stack[i].x1 & 0xffffffff,
        (trace_stack[i].x2 & 0xffffffff) << 32,
         trace_stack[i].x2 & 0xffffffff,
        (trace_stack[i].x3 & 0xffffffff) << 32,
         trace_stack[i].x3 & 0xffffffff);
    }
}
