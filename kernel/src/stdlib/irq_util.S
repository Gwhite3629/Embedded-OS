.globl init_vectors
init_vectors:
    adr x0, vectors
    msr vbar_el1, x0
    ret

.globl enable_interrupts
enable_interrupts:
    msr daifclr, #2
    ret

.globl disable_interrupts
disable_interrupts:
    msr daifset, #2
    ret

.globl interrupt_barrier
interrupt_barrier:
    dsb sy
    ret

