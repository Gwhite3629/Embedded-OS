#include <stdlib.h>
#include <drivers/io.h>
#include <drivers/gic400.h>
#include <drivers/graphics/framebuffer.h>

//irq_t IRQ_LIST[1] = {};

void handle_error(uint64_t x0)
{
	uint64_t x1;
	uint64_t x2;
	uint64_t x3;

	__asm volatile(
		"mrs %[result], esr_el1"
		: [result] "=r"(x1));
	asm volatile("dmb sy");

	__asm volatile(
    	"mrs %[result], elr_el1"
		: [result] "=r"(x2));
	asm volatile("dmb sy");

	__asm volatile(
		"mrs %[result], far_el1"
		: [result] "=r"(x3));
	asm volatile("dmb sy");

	printk("\x1b[1;31mERROR EXCEPTION\x1b[0m %x %x %x %x\n", x0, x1, x2, x3);

	asm volatile("eret");
}

void handle_irq(void)
{
	handle_irq_event(NULL);
}

err_t handle_irq_event(irq_t * q)
{
    err_t ret = E_NOERR;
	uint32_t pending;

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
		draw_rect(0,0,64,16,0xb,1);
		print_screen(4,4,0,"%d", tick_counter);
	}
    return ret;
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

void init_vectors(void)
{
	asm volatile(
		"adr x0, vectors\n"
    	"msr vbar_el1, x0\n");
}

void enable_interrupts(void)
{
    asm volatile(
		"msr daifclr, #2\n");
}

void disable_interrupts(void)
{
	asm volatile(
		"msr daifset, #2\n");
}

void interrupt_barrier(void)
{
	asm volatile(
    	"dsb sy\n");
}
