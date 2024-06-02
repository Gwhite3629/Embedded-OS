#include <stdlib.h/serial.h>
#include <stdlib.h/interrupts.h>
#include "bootscreen.h"

void main()
{
    uart_init();
    uart_write("UART FINISHED\n", 14);
    timer_init();
    uart_write("TIMER FINISHED\n", 15);
    init_vectors();
    enable_interrupts();
    uart_write("INTERRUPTS FINISHED\n", 20);

    printk("\nWaiting for serial port to be ready (press any key)\n");
	uart_getc();

    
	printk("\x1b[2J");
	printk(bootmsg);

//    asm volatile(
//        "msr CPSR_c, #0xDF\n"   /* System mode, like user but has privelege */
//        "mov sp, #0x00002000\n"
//        "msr CPSR_c, #0xD3\n"   /* Back to Supervisor mode */
//        : /* output */
//        : /* input */
//        : "memory");            /* clobbers */

    /* Switch to userspace */

//    asm volatile(
//        "mov x30, %[shell_addr]\n"
//        "eret\n"
//        : /* output */
//        : [shell_addr] "r"((long)&shell) /* input */
//        : "memory");    /* clobbers */

	/* Enter our "shell" */
    while(1);
}
