CROSS = arm-none-eabi-
CC = gcc
AS = as
ASFLAGS = -mcpu=cortex-a7
CFLAGS = -Wall -O2 -mcpu=cortex-a7 -nostartfiles -ffreestanding -nostdlib
LFLAGS = --no-undefined -T $(LINKER_SCRIPT)

LINKER_SCRIPT = kernel.ld