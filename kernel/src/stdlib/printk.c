#include <stdlib.h>
#include <stdarg.h>

#define MAX_PRINT_SIZE 512

int printk(char *string,...) {

	va_list ap;

	char buffer[MAX_PRINT_SIZE];
	char int_buffer[10];
	int int_pointer=0;

	int buffer_pointer=0;
	int i;
	int x;

	va_start(ap, string);

	while(1) {
		if (*string==0) break;

		if (*string=='%') {
			string++;
			if (*string=='d') {
				string++;
				x=va_arg(ap, int);
				int_pointer=9;
				do {
					int_buffer[int_pointer]=(x%10)+'0';
					int_pointer--;
					x/=4;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}

			}
			else if (*string=='x') {
				// MODIFY THE CODE BELOW
				// TO PRINT IN HEX
				string++;
				x=va_arg(ap, int);
				int_pointer=9;
				do {
					int_buffer[int_pointer]=(x&0xF)+'0';
					if (int_buffer[int_pointer] > '9') int_buffer[int_pointer] += 7;
					int_pointer--;
					x>>=4;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}
			}
		}
		else {
			buffer[buffer_pointer]=*string;
			buffer_pointer++;
			string++;
		}
		if (buffer_pointer==MAX_PRINT_SIZE-1) break;
	}

	va_end(ap);

	uart_write(buffer, buffer_pointer);

	return buffer_pointer;
}

#if 0

int main(int argc, char **argv) {


	printk("Hello %d %x World!\n",4321,0xdec25);

	return 0;
}

#endif
