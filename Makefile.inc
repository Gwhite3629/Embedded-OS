#CROSS = ~/cross/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-
CROSS = aarch64-linux-gnu-
CC = gcc
AS = as
ASFLAGS = -mcpu=cortex-a72+nosimd
DISABLE_WARN = -Wno-unused-function -Wno-unused-variable -Wno-attributes
CFLAGS = -Wall -O1 -mcpu=cortex-a72+nosimd -mstrict-align -nostartfiles -ffreestanding -nostdlib $(DISABLE_WARN)
LFLAGS = --no-undefined -T $(LINKER_SCRIPT)

LINKER_SCRIPT = $(srctree)/kernel.ld
