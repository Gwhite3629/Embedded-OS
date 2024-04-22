include Makefile.inc

PHONY := _all

srctree := ..

ktree := $(srctree)/kernel/src
utree := $(srctree)/user

KBUILD_OUTPUT := build
saved-output := $(KBUILD_OUTPUT)
KBUILD_OUTPUT := $(shell mkdir -p $(KBUILD_OUTPUT)\
					&& cd $(KBUILD_OUTPUT)\
					&& /bin/pwd)

KINCLUDE := -I$(srctree)/kernel/include
UINCLUDE := -I$(srctree)/user/include

_all:

export CROSS CC AS CFLAGS LFLAGS ASFLAGS
export KINCLUDE UINCLUDE
export ktree utree

$(if $(KBUILD_OUTPUT),, \
     $(error failed to create output directory "$(saved-output)"))

PHONY += $(MAKECMDGOALS) sub-make
PHONY += all

all: __build_kernel __build_user kernel7.img kernel7l.img

$(filter-out _all sub-make $(ktree)/Makefile, $(MAKECMDGOALS)) _all: sub-make
	@:

sub-make:
	$(MAKE) -C $(KBUILD_OUTPUT) KBUILD_SRC=$(ktree) \
	-f $(ktree)/Makefile $(filter-out _all sub-make,$(MAKECMDGOALS))

PHONY += $(MAKECMDGOALS) __build_kernel

$(filter-out __build_kernel, $(MAKECMDGOALS)): __build_kernel
	@:

__build_kernel:
	for i in $(MAKECMDGOALS); do \
		$(MAKE) -f $(ktree)/Makefile $$i; \
	done

PHONY += $(MAKECMDGOALS) __build_user

$(filter-out __build_user, $(MAKECMDGOALS)): __build_user
	@:

__build_user:
	for i in $(MAKECMDGOALS); do \
		$(MAKE) -f $(utree)/Makefile $$i; \
	done

kernel7l.img:	kernel7.elf
	$(CROSS)objcopy kernel7.elf -O binary kernel7.img

kernel7.elf:	kernel_main.o \
	$(obj) $(obj-u)
	$(CROSS)ld $(LFLAGS) \
		kernel_main.o $(obj) $(obj-u) \
		-Map kernel7.map -o kernel7.elf

clean:
	rm -rf $(saved-output)