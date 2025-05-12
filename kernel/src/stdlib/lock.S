#include "asm_macros.S"

        .globl spin_lock
        .globl spin_unlock

func spin_lock
    mov     w2, #1
    sevl
l1: wfe
l2: ldaxr   w1, [x0]
    cbnz    w1, l1
    stxr    w1, w2, [x0]
    cbnz    w1, l2
    ret
endfunc spin_lock

func spin_unlock
    stlr    wzr, [x0]
    ret
endfunc spin_unlock

