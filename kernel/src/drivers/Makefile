CFLAGS += $(KINCLUDE)

obj := sd.o gic400.o

all: $(obj)

sd.o:
	$(CROSS)$(CC) $(CFLAGS) -o sd.o -c $(ktree)/drivers/sd.c

gic400.o:
	$(CROSS)$(CC) $(CFLAGS) -o gic400.o -c $(ktree)/drivers/gic400.c

.PHONY: all