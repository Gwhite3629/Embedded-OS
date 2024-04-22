CFLAGS += $(KINCLUDE)

obj = boot.o

$(obj):
	$(CROSS)$(AS) $(ASFLAGS) -o $(obj) $(ktree)/boot/boot.S

.PHONY: $(obj)