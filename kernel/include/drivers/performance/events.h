#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <perf/perf.h>

// All events taken from Arm ARM v8-A

/* * * * * * * * * * * * * * * *
 * Common Architectural Events *
 * * * * * * * * * * * * * * * */

#define SW_INCR                 0x0000 // software increment
#define LD_RETIRED              0x0006 // load
#define ST_RETIRED              0x0007 // store
#define INST_RETIRED            0x0008 // instruction
#define EXC_TAKEN               0x0009 // exception taken
#define EXC_RETURN              0x000A // exception return
#define CID_WRITE_RETURNED      0x000B // write to CONTEXTIDR
#define PC_WRITE_RETIRED        0x000C // software change of PC
#define BR_IMMED_RETIRED        0x000D // immediate branch executed
#define BR_RETURN_RETIRED       0x000E // branch taken
#define UNALIGNED_LDST_RETURED  0x000F // unaligned load or store
#define TTBR_WRITE_RETIRED      0x001C // write to TTBR
#define CHAIN                   0x001E // chain pair of events
#define BR_RETIRED              0x0021 // branch
#define EXC_UNDEF               0x0081 // exception, other synchronous
#define EXC_SVC                 0x0082 // supervisor call
#define EXC_PABORT              0x0083 // instruction abort
#define EXC_DABORT              0x0084 // data abort or SError
#define EXC_IRQ                 0x0086 // irq
#define EXC_FIQ                 0x0087 // fiq
#define EXC_SMC                 0x0088 // secure monitor call
#define EXC_HVC                 0x008A // hypervisor call
#define EXC_TRAP_PABORT         0x008B // instruction abort not taken locally
#define EXC_TRAP_DABORT         0x008C // data abort not taken locally
#define EXC_TRAP_OTHER          0x008D // other trap not taken locally
#define EXC_TRAP_IRQ            0x008E // irq not taken locally
#define EXC_TRAP_FIQ            0x008F // fiq not taken locally
#define TRB_WRAP                0x400C //
#define PMU_OVFS                0x400D // PMU overflow
#define TRB_TRIG                0x400E // trace buffer trigger event
#define PMU_HOVFS               0x400F // PMU overflow, el2
#define BR_IMMED_TAKEN_RETIRED  0x8108 // br immediate taken
#define BR_IMMED_SKIP_RETIRED   0x8109 // br immediate not taken
#define BR_IND_TAKEN_RETIRED    0x810A // br indirect taken
#define BR_IND_SKIP_RETIRED     0x810B // br indirect not taken
#define BR_INDNR_TAKEN_RETIRED  0x810C // indirect excluding procedure return, taken
#define BR_INDNR_SKIP_RETIRED   0x810D // indirect excluding procedure return, not taken
#define BR_RETURN_ANY_RETIRED   0x810E // procedure return, taken
#define BR_RETURN_SKIP_RETIRED  0x810F // procedure return, not taken
#define BR_IND_RETIRED          0x811D // indirect branch
#define BR_INDNR_RETIRED        0x811E // indirect excluding procedure return
#define BRB_FILTRATE            0x811F // branch record captured
#define BRNL_INDNR_TAKEN_RETIRED    0x8179 // indirect branch without link excluding procedure return, taken
#define BL_TAKEN_RETIRED        0x817A // branch with link, taken
#define BRNL_TAKEN_RETIRED      0x817B // branch without link, taken
#define BL_IND_TAKEN_RETIRED    0x817C // indirect branch with link, taken
#define BRNL_IND_TAKEN_RETIRED  0x817D // indirect branch without link, taken
#define BL_IMMED_TAKEN_RETIRED  0x817E // direct branch with link, taken
#define BRNL_IMMED_TAKEN_RETIRED    0x817F // direct branch without link, taken
#define BR_UNCOND_RETIRED       0x8180 // unconditional branch
#define BR_COND_RETIRED         0x8181 // conditional branch
#define BR_COND_TAKEN_RETIRED   0x8182 // conditional branch, taken
#define BR_HINT_COND_RETIRED    0x8183 // hinted conditional
#define LD_ANY_RETIRED          0x82A8 // load
#define ST_ANY_RETIRED          0x82A9 // store
#define LDST_ANY_RETIRED        0x82AA // load or store
#define DP_RETIRED              0x82AB // integer data processing

/* * * * * * * * * * * * * * * * * * *
 * Common Micro-Architectural Events *
 * * * * * * * * * * * * * * * * * * */

#define L1I_CACHE_REFILL        0x0001 // level 1 instruction cache refill
#define L1I_TLB_REFILL          0x0002 // level 1 instruction tlb refill
#define L1D_CACHE_REFILL        0x0003 // level 1 data cache refill
#define L1D_CACHE               0x0004 // level 1 data cache access
#define L1D_TLB_REFILL          0x0005 // level 1 data tlb refill
#define BR_MIS_PRED             0x0010 // branch mispredicted or not predicted
#define CPU_CYCLES              0x0011 // cycle
#define BR_PRED                 0x0012 // branch speculatively executed
#define MEM_ACCESS              0x0013 // data memory access
#define L1I_CACHE               0x0014 // level 1 instruction cache access
#define L1D_CACHE_WB            0x0015 // level 1 data cache write-back
#define L2D_CACHE               0x0016 // level 2 data cache access
#define L2D_CACHE_REFILL        0x0017 // level 2 data cache refill
#define L2D_CACHE_WB            0x0018 // level 2 data cache write-back
#define BUS_ACCES               0x0019 // bus access
#define MEMORY_ERROR            0x001A // local memory error
#define INST_SPEC               0x001B // operation speculatively executed
#define BUS_CYCLES              0x001D // bus cycle
#define L1D_CACHE_ALLOCATE      0x001F // level 1 data cache allocation without refill
#define L2D_CACHE_ALLOCATE      0x0020 // level 2 data cache allocation without refill
#define BR_MIS_PRED_RETIRED     0x0022 // branch mispredicted
#define STALL_FRONTEND          0x0023 // no operation sent for execution due to the frontend
#define STALL_BACKEND           0x0024 // no operation sent for execution due to the backend
#define L1D_TLB                 0x0025 // level 1 data tlb access
#define L1I_TLB                 0x0026 // level 1 instruction tlb access
#define L2I_CACHE               0x0027 // level 2 instruction cache access
#define L2I_CACHE_REFILL        0x0028 // level 2 instruction cache refill
#define L3D_CACHE_ALLOCATE      0x0029 // level 3 data cache allocation without refill
#define L3D_CACHE_REFILL        0x002A // level 3 data cache refill
#define L3D_CACHE               0x002B // level 3 data cache access
#define L3D_CACHE_WB            0x002C // level 3 data cache write-back
#define L2D_TLB_REFILL          0x002D // level 2 data tlb refill
#define L2I_TLB_REFILL          0x002E // level 2 instruction tlb refill
#define L2D_TLB                 0x002F // level 2 data tlb access
#define L2I_TLB                 0x0030 // level 2 instruction tlb access
#define REMOTE_ACCESS           0x0031 // access to a remote device
#define LL_CACHE                0x0032 // last level cache access
#define LL_CACHE_MISS           0x0033 // last level cache miss
#define DTLB_WALK               0x0034 // data tlb access with at least one walk
#define ITLB_WALK               0x0035 // instruction tlb access with at least one walk
#define LL_CACHE_RD             0x0036 // last level cache access, read
#define LL_CACHE_MISS_RD        0x0037 // last level cache miss, read
#define REMOTE_ACCES_RD         0x0038 // access to a remote device, read
#define L1D_CACHE_LMISS_RD      0x0039 // level 1 data cache long-latency read miss
#define OP_RETIRED              0x003A // micro-operation architecturally executed
#define OP_SPEC                 0x003B // micro-operation speculatively executed
#define STALL                   0x003C // no operation sent for execution
#define STALL_SLOT_BACKEND      0x003D // no operation sent for execution on a slot due to the backend
#define STALL_SLOT_FRONTEND     0x003E // no operation sent for execution on a slot due to the frontend
#define STALL_SLOT              0x003F // no operation sent for execution on a slot
#define L1D_CACHE_RD            0x0040 // level 1 data cache access, read
#define L1D_CACHE_WR            0x0041 // level 1 data cache access, write
#define L1D_CACHE_REFILL_RD     0x0042 // level 1 data cache refill, read
#define L1D_CACHE_REFILL_WR     0x0043 // level 1 data cache refill, write
#define L1D_CACHE_REFILL_INNER  0x0044 // level 1 data cache refill, inner
#define L1D_CACHE_REFILL_OUTER  0x0045 // level 1 data cache refill, outer
#define L1D_CACHE_WB_VICTIM     0x0046 // level 1 data cache write-back, victim
#define L1D_CACHE_WB_CLEAN      0x0047 // level 1 data cache write-back, cleaning and coherency
#define L1D_CACHE_INVAL         0x0048 // level 1 data cache invalidate
#define L1D_TLB_REFILL_RD       0x004C // level 1 data tlb refill, read
#define L1D_TLB_REFILL_WR       0x004D // level 1 data tlb refill, write
#define L1D_TLB_RD              0x004E // level 1 data tlb access, read
#define L1D_TLB_WR              0x004F // level 1 data tlb access, write
#define L2D_CACHE_RD            0x0050 // level 2 data cache access, read
#define L2D_CACHE_WR            0x0051 // level 2 data cache access, write
#define L2D_CACHE_REFILL_RD     0x0052 // level 2 data cache refill, read
#define L2D_CACHE_REFILL_WR     0x0053 // level 2 data cache refill, write
#define L2D_CACHE_WB_VICTIM     0x0056 // level 2 data cache write-back, victim
#define L2D_CACHE_WB_CLEAN      0x0057 // level 2 data cache write-back, cleaning anc coherency
#define L2D_CACHE_INVAL         0x0058 // level 2 data cache invalidate
#define L2D_TLB_REFILL_RD       0x005C // level 2 data tlb refill, read
#define L2D_TLB_REFILL_WR       0x005D // level 2 data tlb refill, write
#define L2D_TLB_RD              0x005E // level 2 data tlb access, read
#define L2D_TLB_WR              0x005F // level 2 data tlb access, write
#define BUS_ACCESS_RD           0x0060 // bus access, read
#define BUS_ACCESS_WR           0x0061 // bus access, write
#define BUS_ACCESS_SHARED       0x0062 // bus access, normal, cacheable, shareable
#define BUS_ACCESS_NOT_SHARED   0x0063 // bus access, not normal, cacheable, shareable
#define BUS_ACCESS_NORMAL       0x0064 // bus access, normal
#define BUS_ACCESS_PERIPH       0x0065 // bus access, peripheral
#define MEM_ACCESS_RD           0x0066 // data memory access, read
#define MEM_ACCESS_WR           0x0067 // data memory access, write
#define LD_SPEC                 0x0070 // load speculatively executed
#define ST_SPEC                 0x0071 // store speculatively executed
#define LDST_SPEC               0x0072 // load or store speculatively executed
#define DP_SPEC                 0x0073 // integer data processing speculatively executed
#define PC_WRITE_SPEC           0x0076 // software change of PC speculatively executed
#define BR_IMMED_SPEC           0x0078 // immediate branch speculatively executed
#define BR_RETURN_SPEC          0x0079 // procedure return speculatively executed
#define BR_INDIRECT_SPEC        0x007A // indirect branch speculatively executed
#define ISB_SPEC                0x007C // instruction synchronization barrier speculatively executed
#define DSB_SPEC                0x007D // data synchronization barrier speculatively executed
#define DMB_SPEC                0x007E // data memory barrier speculatively executed
#define L3D_CACHE_RD            0x00A0 // level 3 data cache access, read
#define L3D_CACHE_WR            0x00A1 // level 3 data cache access, write
#define L3D_CACHE_REFILL_RD     0x00A2 // level 3 data cache refill, read
#define L3D_CACHE_REFILL_WR     0x00A3 // level 3 data cache refill, write
#define L3D_CACHE_WB_VICTIM     0x00A6 // level 3 data cache write-back, victim
#define L3D_CACHE_WB_CLEAN      0x00A7 // level 3 data cache write-back, cleaning and coherency
#define L3D_CACHE_INVAL         0x00A8 // level 3 data cache invalidate
#define CNT_CYCLES              0x4004 // constant frequency cycles
#define STALL_BACKEND_MEM       0x4005 // memory stall cycles
#define L1I_CACHE_LMISS         0x4006 // level 1 instruction cache long-latency miss
#define L2D_CACHE_LMISS_RD      0x4009 // level 2 data cache long-latency read miss
#define L2I_CACHE_LMISS         0x400A // level 2 instruction cache long-latency miss
#define L3D_CACHE_LMISS_RD      0x400B // level 3 data cache long-latency read miss
#define MEM_ACCESS_CHECKED      0x4024 // checked data memory access
#define MEM_ACCESS_CHECKED_RD   0x4025 // checked data memory access, read
#define MEM_ACCESS_CHECKED_WR   0x4026 // checked data memory access, write
#define PRF_SPEC                0x8087 // prefetch operation speculatively executed
#define BR_IMMED_PRED_RETIRED   0x8110 // predicted immmediate branch
#define BR_IMMED_MIS_PRED_RETIRED   0x8111 // mispredicted immediate branch
#define BR_IND_PRED_RETIRED     0x8112 // predicted indirect branch
#define BR_IND_MIS_PRED_RETIRED 0x8113 // mispredicted indirect branch
#define BR_RETURN_PRED_RETIRED  0x8114 // predicted procedure return branch
#define BR_RETURN_MIS_PRED_RETIRED  0x8115 // mispredicted procedure return branch
#define BR_INDNR_PRED_RETIRED   0x8116 // predicted indirect excluding procedure return
#define BR_INDNR_MIS_PRED_RETIRED   0x8117 // mispredicted indirect excluding procesure return
#define BR_TAKEN_PRED_RETIRED   0x8118 // predicted branch taken
#define BR_TAKEN_MIS_PRED_RETIRED   0x8119 // mispredicted branch taken
#define BR_SKIP_PRED_RETIRED    0x811A // predicted branch not taken
#define BR_SKIP_MIS_PRED_RETIRED    0x811B // mispredicted branch not taken
#define BR_PRED_RETIRED         0x811C // predicted branch
#define INST_FETCH_PERCYC       0x8120 // instruction fetches in progress
#define MEM_ACCESS_RD_PERCYC    0x8121 // data memory reads in progress
#define INST_FETCH              0x8124 // instruction memory access

extern perf_event_t events[];

#endif // _EVENTS_H_