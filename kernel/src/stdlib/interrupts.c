#include <stdlib.h>

err_t handle_irq_event(irq_t * q)
{
    err_t ret = E_NOERR;
    lock(&q->irq_lock);

    ret = q->func(q->irq, q->handler_data);

    unlock(&q->irq_lock);
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