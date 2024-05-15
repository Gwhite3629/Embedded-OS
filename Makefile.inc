CROSS = aarch64-linux-gnu-
CC = gcc
AS = as
ASFLAGS = -mcpu=cortex-a72+nosimd
CFLAGS = -Wall -O2 -mcpu=cortex-a72+nosimd -nostartfiles -ffreestanding -nostdlib
LFLAGS = --no-undefined -T $(LINKER_SCRIPT)

LINKER_SCRIPT = $(srctree)/kernel.ld
