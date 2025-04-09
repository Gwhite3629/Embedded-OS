obj-u = $(utree)/lib

$(obj-u):
	for i in $(obj-u); do \
		$(MAKE) -f $$i/Makefile; \
	done

export obj-u

.PHONY: $(obj-u)