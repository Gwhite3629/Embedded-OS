CFLAGS += $(KINCLUDE)

obj := editor.o

all: $(obj)

editor.o:
	$(CROSS)$(CC) $(CFLAGS) -o editor.o -c $(ktree)/editor/editor.c

.PHONY: all