#include "../../stdlib/time.h"

void wait_cycles(unsigned int n)
{
    if(n) {
        while(n--) {
            asm volatile(
                "nop"
            );
        }
    }
}

void wait_msec(unsigned int n)
{
    register unsigned long f, t, r;
    
    // Get counter frequency
    asm volatile (
        "mrs %0, cntfrq_el0"
        : "=r"(f)
    );
    // Get current counter
    asm volatile (
        "mrs &0, cntpct_el0"
        : "=r"(t)
    );
    unsigned long i = ((f/1000)*n)/1000;

    do {
        asm volatile (
            "mrs %0, cntpct_el0"
            : "=r"(r)
        );
    } while (r - t < i);
}
