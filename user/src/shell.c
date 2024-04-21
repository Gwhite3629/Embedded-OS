#include <stddef.h>
#include <stdint.h>
#include "../../shell.h"
#include "../include/memset.h"
#include "../include/string.h"
#include "../include/accctrl.h"

int putchar(char c) {
    int ret = 0;
    char buffer[1];

    buffer[0] = c;

    ret = serial_write((uint32_t)buffer, 1);

    return ret;
}

uint32_t shell(void)
{
    int ret = 0;
    char ch;
	uint32_t buflen = 0;
	char buf[4096];

    /* Enter our "shell" */

    printf("Entered shell\n");

    memset(buf, '\0', 4096);

	while (1) {
		switch(ch = serial_read()) {
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
				interpret(buf);

                memset(buf, '\0', 4096);
                buflen = 0;
				break;
            // Any non-special character
			default:
                buf[buflen] = ch;
                buflen++;
		}
        // Display character to screen
        ret = putchar(ch);
        if (ret != 0) {
            printf("\nERROR: %d", ret);
        }
	}
}

uint32_t interpret(const char *buf)
{
    // Print command
    if (!strncmp("print", buf, 5)) {
        printf("\nHello", buf);
    // Clear command
    } else if (!strncmp("clear", buf, 5)) {
        printf("\x1b[2J");
    // Anything our shell doesn't know
    } else if (!strncmp("time", buf, 4)) {
        printf("\n%d", time());
    } else {
        printf("\nUnknown Command");
    }

	return 0;
}
