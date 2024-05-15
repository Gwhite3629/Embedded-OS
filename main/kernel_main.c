#include <stdlib.h>
#include <drivers/gic400.h>
#include "../user/src/shell.h"
#include "bootscreen.h"

void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2)
{
    (void) r0;

    timer_init();
    gic400_init((void *)0xFF840000UL);

    enable_interrupts();

    uart_init();

    printk("\nWaiting for serial port to be ready (press any key)\n");
	uart_getc();

    
	printk("\x1b[2J");
	printk(bootmsg);

    asm volatile(
        "msr CPSR_c, #0xDF\n"   /* System mode, like user but has privelege */
        "mov sp, #0x00002000\n"
        "msr CPSR_c, #0xD3\n"   /* Back to Supervisor mode */
        : /* output */
        : /* input */
        : "memory");            /* clobbers */

    /* Switch to userspace */

    asm volatile(
        "mov r0, #0x10\n"
        "msr SPSR, r0\n"
        "mov lr, %[shell_addr]\n"
        "movs pc, lr\n"
        : /* output */
        : [shell_addr] "r"((long)&shell) /* input */
        : "r0", "lr", "memory");    /* clobbers */

	/* Enter our "shell" */
	shell();
}