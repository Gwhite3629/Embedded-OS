CFLAGS += UINCLUDE

obj-u := memset.o printf.o qsort.o string.o syscalls.o

memset.o:	memset.c memset.h
	$(CROSS)$(CC) $(CFLAGS) -o memset.o -c memset.c

printf.o:	printf.c printf.h
	$(CROSS)$(CC) $(CFLAGS) -o printf.o -c printf.c

qsort.o:	qsort.c qsort.h
	$(CROSS)$(CC) $(CFLAGS) -o qsort.o -c qsort.c

string.o:	string.c string.h
	$(CROSS)$(CC) $(CFLAGS) -o string.o -c string.c

syscalls.o:	syscalls.c syscalls.h
	$(CROSS)$(CC) $(CFLAGS) -o syscalls.o -c syscalls.c