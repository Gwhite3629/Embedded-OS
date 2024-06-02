CFLAGS += $(KINCLUDE)

obj-m := kernel_main.o

all: $(obj-m) kernel8.img

kernel_main.o:
	$(CROSS)$(CC) $(CFLAGS) -o kernel_main.o -c $(mtree)/kernel_main.c

kernel8.img: $(obj) $(obj-u)
	$(CROSS)ld $(LFLAGS) \
	boot.o interrupts.o memset.o printk.o \
	proc.o serial.o time.o string.o kernel_main.o irqentry.o \
	-o kernel8.elf
	$(CROSS)objcopy -O binary kernel8.elf kernel8.img

.PHONY: all