CFLAGS += $(UINCLUDE)

obj-u := printf_ulib.o syscalls_ulib.o

all: $(obj-u)

printf_ulib.o:
	$(CROSS)$(CC) $(CFLAGS) -o printf_ulib.o -c $(utree)/lib/printf.c

syscalls_ulib.o:
	$(CROSS)$(CC) $(CFLAGS) -o syscalls_ulib.o -c $(utree)/lib/syscalls.c

.PHONY: all