#include <stdlib/serial.h>
#include <stdlib/interrupts.h>
#include <drivers/gic400.h>
#include "bootscreen.h"

uint32_t shell(void);

void main()
{
    uart_init();
    printk("UART FINISHED\n");
    gic400_init((void *)0xFF840000UL);
    init_vectors();
    interrupt_barrier();
    enable_interrupts();
    printk("INTERRUPTS FINISHED\n");
    timer_init();
    printk("TIMER FINISHED\n");

    printk("\nWaiting for serial port to be ready (press any key)\n");
    uart_getc();

	printk("\x1b[2J");
	printk(bootmsg);

/*
    asm volatile(
        "adr x0, shell\n"
        "msr elr_el3, x0\n"
        "eret\n");
*/

    shell();
	/* Enter our "shell" */
    while(1);
}

int putchar(char c) {
    int ret = 0;
    char buffer[1];

    buffer[0] = c;

    ret = uart_write((uint32_t *)buffer, 1);

    return ret;
}

uint32_t interpret(const char *buf, int buflen)
{
    // Print command
    if (!strncmp("print", buf, 5)) {
        printk("\nHello\n", buf);
    // Clear command
    } else if (!strncmp("clear", buf, 5)) {
        printk("\x1b[2J");
    // Anything our shell doesn't know
    } else if (!strncmp("time", buf, 4)) {
        printk("\n%d\n", tick_counter);
    } else if (buflen == 0) {
        printk("\n");
    } else {
        printk("\nUnknown Command\n");
    }

	return 0;
}

uint32_t shell(void)
{
    int ret = 0;
    char ch;
	uint32_t buflen = 0;
	char buf[4096];

    /* Enter our "shell" */

    printk("Entered shell\n");

    memset(buf, '\0', 4096);

	while (1) {
		switch(ch = uart_getc()) {
            // Backspace or delete
			case(0x8):
			case(0x7f):
                if (buflen > 0) {
				    buflen--;
                    buf[buflen] = '\0';
                }
				break;
            // Newline or carriage return
			case(0xa):
			case(0xd):
				interpret(buf, buflen);

                memset(buf, '\0', 4096);
                buflen = 0;
				break;
            // Any non-special character
			default:
                buf[buflen] = ch;
                buflen++;
		}
        // Display character to screen
        uart_putc(ch);
        if (ret != 0) {
            printk("\nERROR: %d\n", ret);
        }
	}
}

