CFLAGS += KINCLUDE

obj := bitrev.o memset.o qsort.o string.o syscall.o interrupts.o time.o serial.o printk.o

bitrev.o:	bitrev.c stdlib/bitrev.h
	$(CROSS)$(CC) $(CFLAGS) -o bitrev.o -c bitrev.c

memset.o:	memset.c stdlib/memset.h
	$(CROSS)$(CC) $(CFLAGS) -o memset.o -c memset.c

qsort.o:	qsort.c stdlib/qsort.h
	$(CROSS)$(CC) $(CFLAGS) -o qsort.o -c qsort.c

string.o:	string.c stdlib/string.h
	$(CROSS)$(CC) $(CFLAGS) -o string.o -c string.c

syscall.o:	syscall.c stdlib/syscall.h
	$(CROSS)$(CC) $(CFLAGS) -o syscall.o -c syscall.c

interrupts.o:	interrupts.c stdlib/interrupts.h
	$(CROSS)$(CC) $(CFLAGS) -o interrupts.o -c interrupts.c

time.o:	time.c stdlib/time.h
	$(CROSS)$(CC) $(CFLAGS) -o time.o -c time.c

serial.o:	serial.c stdlib/serial.h
	$(CROSS)$(CC) $(CFLAGS) -o serial.o -c serial.c

printk.o:	printk.c stdlib/printk.h
	$(CROSS)$(CC) $(CFLAGS) -o printk.o -c printk.c