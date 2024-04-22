obj-u = $(utree)lib/
obj-u += $(utree)src/

$(obj):
	for i in $(obj); do \
		$(MAKE) -f $$i/Makefile; \
	done
.PHONY: $(obj)