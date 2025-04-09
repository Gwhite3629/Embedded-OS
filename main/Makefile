CFLAGS += $(KINCLUDE)

obj-m := kernel_main.o

all: $(obj-m) kernel8.img

kernel_main.o:
	$(CROSS)$(CC) $(CFLAGS) -o kernel_main.o -c $(mtree)/kernel_main.c

kernel8.img: $(obj) $(obj-u)
	$(CROSS)ld $(LFLAGS) \
	boot.o time.o printk.o serial.o string.o lock.o platform.o \
	interrupts.o irqentry.o gic400.o malloc.o hardware_reserve.o \
	memset.o kernel_main.o ext2.o file.o sd.o proc.o editor.o \
	events.o perf.o counters.o qsort.o mailbox.o mmu.o mmu_a.o \
	list.o inode.o tree.o framebuffer.o bsearch.o part.o strace.o \
	commands.o shell.o \
	-o kernel8.elf
	$(CROSS)objcopy -O binary kernel8.elf kernel8.img

.PHONY: all clean

clean:
	rm ../build/
