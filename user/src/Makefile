CFLAGS += UINCLUDE

obj-u := shell.o

all: $(obj-u)

shell.o:
	$(CROSS)$(CC) $(CFLAGS) -o shell.o -c $(utree)/src/shell.c

.PHONY: all