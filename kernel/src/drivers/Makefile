CFLAGS += KINCLUDE

obj := sd.o gic400.o

sd.o:	sd.c drivers/sd.h
	$(CROSS)$(CC) $(CFLAGS) -o sd.o -c sd.c

gic400.o:	gic400.c drivers/gic400.h
	$(CROSS)$(CC) $(CFLAGS) -o gic400.o -c gic400.c