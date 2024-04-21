CFLAGS += KINCLUDE

obj.o := proc.o

proc.o:	proc.c process/proc.h
	$(CROSS)$(CC) $(CFLAGS) -o proc.o -c proc.c