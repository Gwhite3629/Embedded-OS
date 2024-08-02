CFLAGS += $(KINCLUDE)

obj := hardware_reserve.o mmu.o mmu_a.o malloc.o #mmu_func.o

all: $(obj)

hardware_reserve.o:
	$(CROSS)$(CC) $(CFLAGS) -o hardware_reserve.o -c $(ktree)/memory/hardware_reserve.c

mmu.o:
	$(CROSS)$(CC) $(CFLAGS) -o mmu.o -c $(ktree)/memory/mmu.c

mmu_a.o:
	$(CROSS)$(CC) $(CFLAGS) -o mmu_a.o -c $(ktree)/memory/mmu.S

##mmu_func.o:
##	$(CROSS)$(CC) $(CFLAGS) -o mmu_func.o -c $(ktree)/memory/mmu_func.c

malloc.o:
	$(CROSS)$(CC) $(CFLAGS) -o malloc.o -c $(ktree)/memory/malloc.c

.PHONY: all