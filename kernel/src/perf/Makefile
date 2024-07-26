CFLAGS += $(KINCLUDE)

obj := events.o perf.o

all: $(obj)

events.o:
	$(CROSS)$(CC) $(CFLAGS) -o events.o -c $(ktree)/perf/events.c

perf.o:
	$(CROSS)$(CC) $(CFLAGS) -o perf.o -c $(ktree)/perf/perf.c

.PHONY: all