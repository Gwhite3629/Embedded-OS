CFLAGS += $(KINCLUDE)

obj := strace.o

all: $(obj)

strace.o:
	$(CROSS)$(CC) $(CFLAGS) -o strace.o -c $(ktree)/trace/strace.c

.PHONY: all
