save_context:
    push {r0, r14};
    mov r0, lr;


load_context:
    mov r2, r0
    ldr r0, [r2, #60]
    msr SPSR, r0
    ldmia r2, {r0-r14}
    mov pc, lr

store_context:
    mov    r2, r0
    stmia  r2, {r0-lr}
    add    r2, [r2, #60]
    mrs    r0, SPSR
    stmia  r2, {r0}