CFLAGS += $(KINCLUDE)

obj := boot.o irqentry.o

all: $(obj)

boot.o:
	$(CROSS)$(AS) $(ASFLAGS) -o boot.o $(ktree)/boot/boot.S

irqentry.o:
	$(CROSS)$(AS) $(ASFLAGS) -o irqentry.o $(ktree)/boot/irqentry.S

.PHONY: $(obj)