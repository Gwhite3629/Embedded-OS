CFLAGS += $(KINCLUDE)

obj := ext2.o file.o inode.o

all: $(obj)

ext2.o:
	$(CROSS)$(CC) $(CFLAGS) -o ext2.o -c $(ktree)/fs/ext2/ext2.c

file.o:
	$(CROSS)$(CC) $(CFLAGS) -o file.o -c $(ktree)/fs/ext2/file.c

inode.o:
	$(CROSS)$(CC) $(CFLAGS) -o inode.o -c $(ktree)/fs/ext2/inode.c

.PHONY: all