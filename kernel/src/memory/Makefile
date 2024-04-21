CFLAGS += KINCLUDE

obj.o := hardware_reserve.o mmu.o mmu_func.o malloc.o

hardware_reserve:	hardware_reserve.c memory/hardware_reserve.h
	$(CROSS)$(CC) $(CFLAGS) -o hardware_reserve.o -c hardware_reserve.c

mmu.o:	mmu.c memory/mmu.h
	$(CROSS)$(CC) $(CFLAGS) -o mmu.o -c mmu.c

mmu_func.o:	mmu_func.c memory/mmu_func.h
	$(CROSS)$(CC) $(CFLAGS) -o mmu_func.o -c mmu_func.c

malloc.o:	malloc.c memory/malloc.h
	$(CROSS)$(CC) $(CFLAGS) -o malloc.o -c malloc.c