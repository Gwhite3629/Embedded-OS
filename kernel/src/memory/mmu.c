#include <stdlib/err.h>
#include <stdlib/printk.h>
#include <memory/hardware_reserve.h>
#include <process/proc.h>
#include <memory/mmu.h>
#include <drivers/mailbox.h>
#include <drivers/graphics/framebuffer.h>

#include <stdint.h>

/*
unsigned long map_table(unsigned long *table, unsigned long shift, unsigned long va, int *new_table)
{
    unsigned long index = va >> shift;
    index = index & (TABLE_SIZE - 1);
    if (!table[index]) {
        *new_table = 1;
        unsigned long next_level_table = reserve(1, MEM_K);
        unsigned long entry = next_level_table | MM_TYPE_TABLE;
        table[index] = entry;
        return next_level_table;
    } else {
        *new_table = 0;
    }
    return table[index] & PAGE_MASK;
}

void map_table_entry(unsigned long *LVL0, unsigned long va, unsigned long pa)
{
    unsigned long index = va >> PAGE_SHIFT;
    index = index & (TABLE_SIZE - 1);
    unsigned long entry = pa | MM_PTE_FLAGS;
    LVL0[index] = entry;
}

void map_page(proc_t *p, unsigned long va, unsigned long page)
{
    unsigned long LVL3;
    if (!p->mm.LVL3) {
        p->mm.LVL3 = get_kernel_page();
        p->mm.n_kernel_pages++;
    }
    LVL3 = p->mm.LVL3;
    int new_table;
    unsigned long LVL2 = map_table((unsigned long *)(LVL3 + VA_START), LVL3_SHIFT, va, &new_table);
    if (new_table) {
        p->mm.n_kernel_pages++;
    }
    unsigned long LVL1 = map_table((unsigned long *)(LVL2 + VA_START), LVL2_SHIFT, va, &new_table);
    if (new_table) {
        p->mm.n_kernel_pages++;
    }
    unsigned long LVL0 = map_table((unsigned long *)(LVL1 + VA_START), LVL1_SHIFT, va, &new_table);
    if (new_table) {
        p->mm.n_kernel_pages++;
    }
    map_table_entry((unsigned long *)(LVL0 + VA_START), va, page);
    p->mm.n_user_pages++;
}

unsigned long get_kernel_page()
{
    unsigned long page = reserve(1, MEM_K);
    if (page == 0) {
        return 0;
    }
    return page + VA_START;
}

unsigned long get_user_page(proc_t *p, unsigned long va)
{
    unsigned long page = reserve(1, MEM_U);
    if (page == 0) {
        return 0;
    }
    map_page(p, va, page);
    return page + VA_START;
}

void free_page(unsigned long page)
{
    int32_t r = relinquish(page, 1);
    if (r != 0) {
        return;
    }
}
*/

typedef union {
	struct {
		uint64_t EntryType : 2;				// @0-1		1 for a block table, 3 for a page table
			
        /* These are only valid on BLOCK DESCRIPTOR */
		uint64_t MemAttr : 4;			    // @2-5
		enum {
			STAGE2_S2AP_NOREAD_EL0 = 1,	    //			No read access for EL0
			STAGE2_S2AP_NO_WRITE = 2,	    //			No write access
		} S2AP : 2;						    // @6-7
		enum {
			STAGE2_SH_OUTER_SHAREABLE = 2,	//			Outter shareable
			STAGE2_SH_INNER_SHAREABLE = 3,	//			Inner shareable
		} SH : 2;						    // @8-9
		uint64_t AF : 1;				    // @10		Accessable flag

		uint64_t _reserved11 : 1;			// @11		Set to 0
		uint64_t Address : 36;				// @12-47	36 Bits of address
		uint64_t _reserved48_51 : 4;		// @48-51	Set to 0
		uint64_t Contiguous : 1;			// @52		Contiguous
		uint64_t _reserved53 : 1;			// @53		Set to 0
		uint64_t XN : 1;					// @54		No execute if bit set
		uint64_t _reserved55_58 : 4;		// @55-58	Set to 0
		
		uint64_t PXNTable : 1;				// @59      Never allow execution from a lower EL level 
		uint64_t XNTable : 1;				// @60		Never allow translation from a lower EL level
		enum {
			APTABLE_NOEFFECT = 0,			// No effect
			APTABLE_NO_EL0 = 1,				// Access at EL0 not permitted, regardless of permissions in subsequent levels of lookup
			APTABLE_NO_WRITE = 2,			// Write access not permitted, at any Exception level, regardless of permissions in subsequent levels of lookup
			APTABLE_NO_WRITE_EL0_READ = 3	// Write access not permitted,at any Exception level, Read access not permitted at EL0.
		} APTable : 2;						// @61-62	AP Table control .. see enumerate options
		uint64_t NSTable : 1;				// @63		Secure state, for accesses from Non-secure state this bit is RES0 and is ignored
	};
	uint64_t Raw64;							// @0-63	Raw access to all 64 bits via this union
} VMSAv8_64_DESCRIPTOR;

static uint64_t __attribute__((aligned(TLB_ALIGN))) PT_identity1[PT_NUM] = {0};
static uint64_t __attribute__((aligned(TLB_ALIGN))) PT_virtual1[PT_NUM] = {0};
static uint64_t __attribute__((aligned(TLB_ALIGN))) PT_virtual2[PT_NUM] = {0};
static VMSAv8_64_DESCRIPTOR __attribute__((aligned(TLB_ALIGN))) PT_identity2[4096] = { 0 };
static VMSAv8_64_DESCRIPTOR __attribute__((aligned(TLB_ALIGN))) PT_virtual3[512] = { 0 };


void identity_map(void)
{
    uint32_t base;
    get_vc_address();
    uint32_t vc_val = vc_base_address / PT_SIZE;

    printk("VC Address %x\n", vc_base_address);

    // Region 0 -> VC mem
    for (base = 0; base < (vc_val - 1); base++) {
        PT_identity2[base] = (VMSAv8_64_DESCRIPTOR){
            .Address = (uintptr_t)base << (21 - 12),
            .AF = 1,
            .SH = STAGE2_SH_INNER_SHAREABLE,
            .MemAttr = MT_NORMAL_NC,
            .EntryType = 1,
        };
    }

    PT_identity2[(vc_val - 1)] = (VMSAv8_64_DESCRIPTOR){
        .Address = (uintptr_t)(vc_val - 1) << (21 - 12),
        .AF = 1,
        .MemAttr = MT_DEVICE_NGNRNE,
        .EntryType = 1,
    };
    mbox = ((vc_val - 1) << 21);
    printk("MAILBOX ADDRESS: %x\n", mbox);

    // Region VC mem -> 0xFE000000
    for (; base < (2048 - 16); base++) {
        PT_identity2[base] = (VMSAv8_64_DESCRIPTOR){
            .Address = (uintptr_t)base << (21 - 12),
            .AF = 1,
            .MemAttr = MT_NORMAL_NC,
            .EntryType = 1,
        };
    }

    for (; base < 2048; base++) {
        PT_identity2[base] = (VMSAv8_64_DESCRIPTOR){
            .Address = (uintptr_t)base << (21 - 12),
            .AF = 1,
			.MemAttr = MT_DEVICE_NGNRNE,
			.EntryType = 1,
        };
    }

    for (; base < 4096; base++) {
        PT_identity2[base] = (VMSAv8_64_DESCRIPTOR){
            .Address = (uintptr_t)base << (21 - 12),
            .AF = 1,
			.MemAttr = MT_NORMAL,
			.EntryType = 1,
        };
    }

    uint32_t n_fb_pages = (fb.buf_size / PT_SIZE) + 1;
    uint32_t fb_start = ((uint32_t)fb.buf) / PT_SIZE;
    uint32_t map_start = (vc_val - 1) - n_fb_pages;

    uint32_t offset = ((uint32_t)fb.buf) % PT_SIZE;

    for (uint32_t i = 0; i < n_fb_pages; i++) {
        PT_identity2[map_start + i] = (VMSAv8_64_DESCRIPTOR){
            .Address = (uintptr_t)(fb_start + i) << (21 - 12),
            .AF = 1,
            .MemAttr = MT_DEVICE_NGNRNE,
            .EntryType = 1,
        };
    }

    fb.buf = (unsigned char *)((map_start << 21) + (offset));

    printk("\x1b[1;32mCREATED FRAMEBUFFER EXCEPTION\x1b[1;0m\n");
    printk("\tNEW FB ADDRESS: %x\n", fb.buf);
    printk("\tn_pages: %d\n", n_fb_pages);

    PT_identity1[0]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[0] | 3;
    PT_identity1[1]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[512] | 3;
    PT_identity1[2]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[1024] | 3;
    PT_identity1[3]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[1536] | 3;
    PT_identity1[4]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[2048] | 3;
    PT_identity1[5]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[2560] | 3;
    PT_identity1[6]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[3072] | 3;
    PT_identity1[7]     = (0x8000000000000000) | (uintptr_t)&PT_identity2[3584] | 3;

    PT_virtual2[511]    = (0x8000000000000000) | (uintptr_t)&PT_virtual3[0] | 3;

    PT_virtual1[511]    = (0x8000000000000000) | (uintptr_t)&PT_virtual2[0] | 3;

}

uint64_t virtualmap(uint64_t phys_addr, uint8_t memattrs)
{
    uint64_t addr = 0;
    for (int i = 0; i < 512; i++) {
        if (PT_virtual3[i].Raw64 == 0) {
            uint64_t offset;
            PT_virtual3[i] = (VMSAv8_64_DESCRIPTOR){
                .Address = (uintptr_t)phys_addr << (21 - 12),
                .AF = 1,
                .MemAttr = memattrs,
                .EntryType = 3,
            };
            __asm volatile("dmb sy" ::: "memory");
            offset = ((512 - i) * 4096) - 1;
            addr = 0xFFFFFFFFFFFFFFFFul;
            addr = addr - offset;
            return (addr);
        }
    }
    return (addr);
}

void enable_MMU(void)
{
    enable_mmu_tables(&PT_identity1[0], &PT_virtual1[0]);
}