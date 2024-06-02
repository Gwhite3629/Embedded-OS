CFLAGS += $(KINCLUDE)
ASFLAGS += $(KINCLUDE)

obj := boot.o irqentry.o

all: $(obj)

boot.o:
	$(CROSS)$(CC) $(CFLAGS) -o boot.o -c $(ktree)/boot/boot.S

irqentry.o:
	$(CROSS)$(AS) $(ASFLAGS) -o irqentry.o $(ktree)/boot/irqentry.S

.PHONY: $(obj)