/* Barebones ARM/Raspberry Pi Standalone (kernel) linker script */
/* http://wiki.osdev.org/Raspberry_Pi_Bare_Bones		*/
 
SECTIONS
{
	/************************ Text Section ********************/
	.text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }

	/****************** Read-Only Data Section ********************/
	.rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }


	/****************** Data Section ********************/
	PROVIDE(_data = .);
	.data : { *(.data .data.* .gnu.linkonce.d*) }
 
	/****************** BSS Section ********************/
	.bss (NOLOAD) :
	{
		. = ALIGN(16);
		__bss_start = .;
		*(.bss .bss.*)
		*(COMMON)
		__bss_end = .;
		__bss_size = (__bss_end - __bss_start)>>3;
	}
	pg_dir = .;
	.cpu1Stack :
	{
		. = ALIGN(16);
		__stack_start = .;
		. = . + 512;
		__cpu1_stack = .;
	}
	/****************** End of executable **************/
	__end = .;

	/DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
