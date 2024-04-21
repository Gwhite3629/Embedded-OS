CFLAGS += KINCLUDE

obj := syscalls.o

syscalls.o:	syscalls.c syscall/syscalls.h
	$(CROSS)$(CC) $(CFLAGS) -o syscalls.o -c syscalls.c