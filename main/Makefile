CFLAGS += $(KINCLUDE)

obj-m := kernel_main.o

all: $(obj-m) kernel7.img kernel7l.img

kernel_main.o:
	$(CROSS)$(CC) $(CFLAGS) -o kernel_main.o -c $(mtree)/kernel_main.c

kernel7.img:
	$(obj) $(obj-u)
		$(CROSS)ld $(LFLAGS) \
			kernel_main.o ./*.o \
			-Map kernel7.map -o kernel7.elf

kernel7l.img:
	$(CROSS)objcopy kernel7.elf -O binary kernel7.img

.PHONY: all