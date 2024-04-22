CFLAGS += $(KINCLUDE)

obj := syscalls.o swi_handler.o

all: $(obj)

syscalls.o:	
	$(CROSS)$(CC) $(CFLAGS) -o syscalls.o -c $(ktree)/syscall/syscalls.c

swi_handler.o:
	$(CROSS)$(AS) $(ASFLAGS) -o swi_handler.o $(ktree)/syscall/swi_handler.S

.PHONY: all