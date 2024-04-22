#include <memory/mmu_func.h>

void tlb_invalidate_all(void) {
    uint32_t reg=0;

	/* TLBIALL */
	asm volatile("mcr p15, 0, %0, c8, c7, 0"
		: : "r" (reg) : "memory");
}

void icache_invalidate_all(void) {
	uint32_t reg=0;

	/* ICIALLU */
	asm volatile("mcr p15, 0, %0, c7, c5, 1"
		: : "r" (reg) : "memory");
}

void disable_l1_dcache(void) {
	// Disable L1 Caches.
	asm volatile(
	"mrc	p15, 0, r1, c1, c0, 0	// Read SCTLR.\n"
	"bic	r1, r1, #(0x1 << 2)	// Disable D Cache.\n"
	"mcr	p15, 0, r1, c1, c0, 0	// Write SCTLR.\n"
	);
}

void invalidate_l1_dcache(void) {

	asm volatile(
	"push	{r0 - r12}\n"
	"mrc	p15, 1, r0, c0, c0, 1		/* read CLIDR */\n"
	"ands	r3, r0, #0x7000000\n"
	"mov	r3, r3, lsr #23			/* cache level value (naturally aligned) */\n"
	"beq	finished\n"
	"mov	r10, #0				/* start with level 0 */\n"
"loop1:\n"
	"add	r2, r10, r10, lsr #1		/* work out 3xcachelevel */\n"
	"mov	r1, r0, lsr r2			/* bottom 3 bits are the Cache type for this level */\n"
	"and	r1, r1, #7			/* get those 3 bits alone */\n"
	"cmp	r1, #2\n"
	"blt	skip				/* no cache or only instruction cache at this level */\n"
	"mcr	p15, 2, r10, c0, c0, 0		/* write the Cache Size selection register */\n"
	"isb					/* isb to sync the change to the CacheSizeID reg */\n"
	"mrc	p15, 1, r1, c0, c0, 0		/* reads current Cache Size ID register */\n"
	"and	r2, r1, #7			/* extract the line length field */\n"
	"add	r2, r2, #4			/* add 4 for the line length offset (log2 16 bytes) */\n"
	"ldr	r4, =0x3ff\n"
	"ands	r4, r4, r1, lsr #3		/* r4 is the max number on the way size (right aligned) */\n"
	"clz	r5, r4				/* r5 is the bit position of the way size increment */\n"
	"ldr	r7, =0x7fff\n"
	"ands	r7, r7, r1, lsr #13		/* r7 is the max number of the index size (right aligned) */\n"
"loop2:\n"
	"mov	r9, r4				/* r9 working copy of the max way size (right aligned) */\n"
"loop3:\n"
	"orr	r11, r10, r9, lsl r5		/* factor in the way number and cache number into r11 */\n"
	"orr	r11, r11, r7, lsl r2		/* factor in the index number */\n"
//	"mcr	p15, 0, r11, c7, c6, 2		/* invalidate by set/way */\n"
	"mcr	p15, 0, r11, c7, c14, 2		/* clean+invalidate by set/way */\n"
	"subs	r9, r9, #1			/* decrement the way number */\n"
	"bge	loop3\n"
	"subs	r7, r7, #1			/* decrement the index */\n"
	"bge	loop2\n"
"skip:\n"
	"add	r10, r10, #2			/* increment the cache number */\n"
	"cmp	r3, r10\n"
	"bgt	loop1\n"
"finished:\n"
	"mov	r10, #0				/* swith back to cache level 0 */\n"
	"mcr	p15, 2, r10, c0, c0, 0		/* select current cache level in cssr */\n"
	"dsb\n"
	"isb\n"
	"pop	{r0-r12}\n"
	);
}

void enable_l1_dcache(void) {

	/* still issues with this on pi3 */

	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 12: enable dcache */
	asm volatile( "orr r0, r0, #4" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
}

void flush_icache(void) {

	uint32_t reg=0;

//; Enter this code with <Rx> containing the new 32-bit instruction.
// STR <Rx>, [instruction location]
// DCCMVAU [instruction location] ; Clean data cache by MVA to point of unification
// DSB
//; Ensure visibility of the data cleaned from the cache
// ICIMVAU [instruction location] ; Invalidate instruction cache by MVA to PoU
// BPIMVAU [instruction location] ; Invalidate branch predictor by MVA to PoU
//DSB ; Ensure completion of the invalidations
// ISB ; Synchronize fetched instruction stream


//	ICIALLU c c7 0 c5 0 32-bit WO Instruction cache invalidate all
//	BPIALL c c7 0 c5 6 32-bit WO Branch predictor invalidate all -

	asm volatile(	"mcr p15, 0, %0, c7, c5, 0\n"	// ICIALLU
			"mcr p15, 0, %0, c7, c5, 6\n"	// BPIALL
			"dsb\n"
			"isb\n"
			: : "r" (reg): "cc");

}

void flush_dcache(uint32_t start_addr, uint32_t end_addr) {

	uint32_t mva;

	// 1559
	// DCCIMVAC, Data Cache Clean and Invalidate by MVA to PoC, VMSA
	// 1643, 1496
	// DCCIMVAC c c7 0 c14 1
	// DCCIMVAU c c7 0 c14 1

	// clean and invalidate data or unified cache line
	// DCCIMVAC c c7 0 c14 1

//	invalidate_l1_dcache();
#if 1
	for(mva=(start_addr&0xfffffff0);mva<=(end_addr&0xfffffff0);mva+=16) {

		asm volatile("mcr p15, 0, %0, c7, c14, 1\n"
			: : "r" (mva) : "memory");
	}
#endif
	asm volatile("dsb\n"  // DSB
		: : : "memory");
	asm volatile("isb\n"  // ISB
		: : : "memory");

}
