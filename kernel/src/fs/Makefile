CFLAGS += $(KINCLUDE)

obj := fat32.o file.o

all: $(obj)

fat32.o:
	$(CROSS)$(CC) $(CFLAGS) -o fat32.o -c $(ktree)/fs/fat32.c

file.o:
	$(CROSS)$(CC) $(CFLAGS) -o file.o -c $(ktree)/fs/file.c

.PHONY: all