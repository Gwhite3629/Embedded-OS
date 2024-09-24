CFLAGS += $(KINCLUDE)

obj := sd.o gic400.o counters.o mailbox.o platform.o framebuffer.o

all: $(obj)

sd.o:
	$(CROSS)$(CC) $(CFLAGS) -o sd.o -c $(ktree)/drivers/sd.c

gic400.o:
	$(CROSS)$(CC) $(CFLAGS) -o gic400.o -c $(ktree)/drivers/gic400.c

counters.o:
	$(CROSS)$(CC) $(CFLAGS) -o counters.o -c $(ktree)/drivers/performance/counters.c

platform.o:
	$(CROSS)$(CC) $(CFLAGS) -o platform.o -c $(ktree)/drivers/platform.c

mailbox.o:
	$(CROSS)$(CC) $(CFLAGS) -o mailbox.o -c $(ktree)/drivers/mailbox.c

framebuffer.o:
	$(CROSS)$(CC) $(CFLAGS) -o framebuffer.o -c $(ktree)/drivers/graphics/framebuffer.c

.PHONY: all