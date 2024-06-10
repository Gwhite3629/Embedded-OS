#include "../../include/stdlib.h"
#include "../../include/process/proc.h"
#include "../../include/memory/malloc.h"
#include "../../include/syscalls/syscalls.h"
#include "../../include/fs/file.h"
#include "../../include/stdlib/time.h"

err_t swi_handler_c(uint64_t r0, uint64_t r1, uint64_t r2, uint64_t r3)
{
    register long r7 asm ("r7");

    err_t ret = 0;

    switch(r7) {
        case SYS_PEXIT:
            proc_exit((proc_t *)r0, E_NOERR);
            break;

        case SYS_SREAD:
            ret = uart_getc();
            break;

        case SYS_SWRITE:
            uart_write((void *)r0, (unsigned int)r1);
            break;

        case SYS_FREAD:
            ret = f_read((FILE *)r0, (char *)r1, (unsigned int)r2, (unsigned int  *)r3);
            break;

        case SYS_FWRITE:
            ret = f_write((FILE *)r0, (const char *)r1, (unsigned int)r2, (unsigned int *)r3);
            break;

        case SYS_FOPEN:
            ret = f_open((FILE *)r0, (const char *)r1, (uint8_t)r2);
            break;

        case SYS_FCLOSE:
            ret = f_close((FILE *)r0);
            break;

        case SYS_PSTART:
            break;

        case SYS_MALLOC:
            r0 = (uint64_t)alloc(current_proc->heap, (int)r1);
            if ((uint32_t *)r0 == NULL) {
                ret = E_NOMEM;
            }
            break;

        case SYS_FREE:
            cull(current_proc->heap, (uint32_t *)r0);
            ret = E_NOERR;
            break;

        case SYS_TIME:
            if (r0 == 0) {
                printk("Invalid write address\n");
                ret = E_INVALID;
            } else {
                (*((uint32_t *)(r0))) = (uint32_t)tick_counter;
                ret = tick_counter;
            }
            break;
    }

    return ret;
};