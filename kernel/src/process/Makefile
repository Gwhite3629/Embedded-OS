#KINCLUDE := $(if $(KINCLUDE),$(KINCLUDE),../../include/)
#ktree := $(if $(ktree),$(ktree),..)
CFLAGS += $(KINCLUDE)

obj := proc.o

all: $(obj)

proc.o:
	$(CROSS)$(CC) $(CFLAGS) -o proc.o -c $(ktree)/process/proc.c

.PHONY: all