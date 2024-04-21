CFLAGS += KINCLUDE

obj := fat32.o file.o

fat32.o:	fat32.c	fs/fat32.h
	$(CROSS)$(CC) $(CFLAGS) -o fat32.o -c fat32.c

file.o:	file.c fs/file.h
	$(CROSS)$(CC) $(CFLAGS) -o file.o -c file.c