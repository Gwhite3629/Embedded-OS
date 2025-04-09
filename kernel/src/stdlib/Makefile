CFLAGS += $(KINCLUDE)
obj := bsearch.o list.o tree.o lock.o rbtree.o bitrev.o memset.o qsort.o string.o interrupts.o time.o serial.o printk.o shell.o commands.o

all: $(obj)

bsearch.o:
	$(CROSS)$(CC) $(CFLAGS) -o bsearch.o -c $(ktree)/stdlib/bsearch.c

lock.o:
	$(CROSS)$(CC) $(CFLAGS) -o lock.o -c $(ktree)/stdlib/lock.c

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

#syscall.o:
#	$(CROSS)$(CC) $(CFLAGS) -o syscall.o -c $(ktree)/stdlib/syscall.c

interrupts.o:
	$(CROSS)$(CC) $(CFLAGS) -o interrupts.o -c $(ktree)/stdlib/interrupts.c

time.o:
	$(CROSS)$(CC) $(CFLAGS) -o time.o -c $(ktree)/stdlib/time.c

serial.o:
	$(CROSS)$(CC) $(CFLAGS) -o serial.o -c $(ktree)/stdlib/serial.c

printk.o:
	$(CROSS)$(CC) $(CFLAGS) -o printk.o -c $(ktree)/stdlib/printk.c

tree.o:
	$(CROSS)$(CC) $(CFLAGS) -o tree.o -c $(ktree)/stdlib/tree.c

list.o:
	$(CROSS)$(CC) $(CFLAGS) -o list.o -c $(ktree)/stdlib/list.c

shell.o:
	$(CROSS)$(CC) $(CFLAGS) -o shell.o -c $(ktree)/stdlib/shell.c

commands.o:
	$(CROSS)$(CC) $(CFLAGS) -o commands.o -c $(ktree)/stdlib/commands.c

.PHONY: all