CFLAGS += $(KINCLUDE)
obj := rbtree.o bitrev.o memset.o qsort.o string.o syscall.o interrupts.o time.o serial.o printk.o

all: $(obj)

rbtree.o:
	$(CROSS)$(CC) $(CFLAGS) -o rbtree.o -c $(ktree)/stdlib/rbtree.c

bitrev.o:
	$(CROSS)$(CC) $(CFLAGS) -o bitrev.o -c $(ktree)/stdlib/bitrev.c

memset.o:
	$(CROSS)$(CC) $(CFLAGS) -o memset.o -c $(ktree)/stdlib/memset.c

qsort.o:
	$(CROSS)$(CC) $(CFLAGS) -o qsort.o -c $(ktree)/stdlib/qsort.c

string.o:
	$(CROSS)$(CC) $(CFLAGS) -o string.o -c $(ktree)/stdlib/string.c

syscall.o:
	$(CROSS)$(CC) $(CFLAGS) -o syscall.o -c $(ktree)/stdlib/syscall.c

interrupts.o:
	$(CROSS)$(CC) $(CFLAGS) -o interrupts.o -c $(ktree)/stdlib/interrupts.c

time.o:
	$(CROSS)$(CC) $(CFLAGS) -o time.o -c $(ktree)/stdlib/time.c

serial.o:
	$(CROSS)$(CC) $(CFLAGS) -o serial.o -c $(ktree)/stdlib/serial.c

printk.o:
	$(CROSS)$(CC) $(CFLAGS) -o printk.o -c $(ktree)/stdlib/printk.c

.PHONY: all