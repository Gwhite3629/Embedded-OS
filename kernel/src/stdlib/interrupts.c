#include <stdlib.h>
#include <drivers/io.h>
#include <drivers/gic400.h>
#include <drivers/graphics/framebuffer.h>
#include <trace/strace.h>

//irq_t IRQ_LIST[1] = {};

static volatile int tick_counter;

void init_tickcounter() {tick_counter = 0;}

int get_tickcounter() {return tick_counter;}

inline void handle_error(uint64_t x0)
{
	uint64_t x1 = 0;
	uint64_t x2 = 0;
	uint64_t x3 = 0;
    uint64_t x4 = 0;
    uint64_t sp = 0;

	__asm volatile(
		"mrs %[result], esr_el1"
		: [result] "=r"(x1));
	asm volatile("dmb sy");

	__asm volatile(
    	"mrs %[result], elr_el1"
		: [result] "=r"(x2));
	asm volatile("dmb sy");

    __asm volatile(
        "mrs %[result], sctlr_el1"
        : [result] "=r"(x3));
    asm volatile("dmb sy");

	__asm volatile(
		"mrs %[result], spsr_el1"
		: [result] "=r"(x4));
	asm volatile("dmb sy");

    __asm volatile(
        "mov %[result], sp"
        : [result] "=r"(sp));
    asm volatile("dmb sy");

	printk("\x1b[1;31mERROR EXCEPTION\x1b[0m \
    \nint_ent:   %8x%8x \
    \nesr_el1:   %8x%8x \
    \nelr_el1:   %8x%8x \
    \nsctlr_el1: %8x%8x \
    \nspsr_el1:  %8x%8x \
    \nsp:        %8x%8x\n", \
    x0>>32,x0,x1>>32,x1,x2>>32,x2,x3>>32,x3,x4>>32,x4,sp>>32,sp);

    while(1) {};

	return;
}

void noinline handle_irq(void)
{
    err_t ret = E_NOERR;
	uint32_t pending;
	uint32_t x;

	pending = chip_read(IRQ0_PENDING2);
    //acquire(&q->irq_lock);

    //ret = q->func(q->irq, q->handler_data);

    //release(&q->irq_lock);

	if ((pending & 0x1)!=0x1) {
		printk("Unknown Interrupt %x\n", pending);
	} else {
		tick_counter++;
        chip_write(0x1, TIMER_IRQ_CLEAR);
        chip_write(0xffffffffUL, GICD_ICPENDR0);
        __asm volatile(
			"mov %[result], sp"
			: [result] "=r"(x));
		asm volatile("dmb sy");
		draw_rect(0,0,95,15,0xCEC655,1);
		struct chr_dat tc = {4,4,4,4,0};
		print_screen(&tc,"%d", tick_counter);
		draw_rect(0,16,95,31,0xCEC655,1);
		struct chr_dat sp = {4,19,4,19,0};
		print_screen(&sp,"0x%8x", x);
        //dump_trace(__new);
        //__new = 0;
    }
}

void __UNDEF__ undefined_handler(void) {

	printk("Undefined instruction!\n");
	while(1);

}

void __ABORT__ prefetch_handler(void) {

	printk("Unexpected prefetch handler interrupt!\n");
	while(1);

}

void __ABORT__ data_handler(void) {

	printk("Unexpected data handler interrupt!\n");
	while(1);

}

void __FIQ__ fiq_handler(void) {

	printk("Unexpected FIQ interrupt!\n");
	while(1);

}

void __SVC__ reset_error(void) {

	printk("Reset triggered!\n");
	while(1);

}

void noinline init_vectors(void)
{
	asm volatile(
		"adr x0, vectors\n"
    	"msr vbar_el1, x0\n");
}

void noinline enable_interrupts(void)
{
    asm volatile(
		"msr daifclr, #7\n");
}

void noinline disable_interrupts(void)
{
    interrupt_barrier();
    asm volatile(
		"msr daifset, #7\n");
}

void noinline memory_barrier(void)
{
    asm volatile(
        "dmb sy\n");
}

void noinline interrupt_barrier(void)
{
	asm volatile(
    	"dsb sy\n");
}

void noinline instruction_barrier(void)
{
    asm volatile(
        "isb\n");
}
