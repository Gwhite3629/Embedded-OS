CFLAGS += UINCLUDE

obj-u := memset.o printf.o qsort.o string.o syscalls.o

all: $(obj-u)

memset.o:
	$(CROSS)$(CC) $(CFLAGS) -o memset.o -c $(utree)/lib/memset.c

printf.o:
	$(CROSS)$(CC) $(CFLAGS) -o printf.o -c $(utree)/lib/printf.c

qsort.o:
	$(CROSS)$(CC) $(CFLAGS) -o qsort.o -c $(utree)/lib/qsort.c

string.o:
	$(CROSS)$(CC) $(CFLAGS) -o string.o -c $(utree)/lib/string.c

syscalls.o:
	$(CROSS)$(CC) $(CFLAGS) -o syscalls.o -c $(utree)/lib/syscalls.c

.PHONY: all