obj = $(ktree)/boot
obj += $(ktree)/stdlib
obj += $(ktree)/drivers
obj += $(ktree)/memory
obj += $(ktree)/process
obj += $(ktree)/fs
obj += $(ktree)/syscall

$(obj):
	for i in $(obj); do \
		$(MAKE) -f $$i/Makefile; \
	done
.PHONY: $(obj)