#include <drivers/performance/events.h>
#include <perf/perf.h>

static perf_event_t events[] =
{
    {
        "SW_INC",
        0x0000,
        -1
    },
    {
        "L1I_Cache_Refill",
        0x0001,
        -1
    },
    {
        "L1I_TLB_Refill",
        0x0002,
        -1
    },
    {
        "L1D_Cache_Refill",
        0x0003,
        -1
    },
    {
        "L1D_Cache_Access",
        0x0004,
        -1
    },
    {
        "L1D_TLB_Refill",
        0x0005,
        -1
    },
    {
        "Load",
        0x0006,
        -1
    },
    {
        "Store",
        0x0007,
        -1
    },
    {
        "Instruction",
        0x0008,
        -1
    },
    {
        "Exception_Taken",
        0x0009,
        -1
    },
    {
        "Exception_Return",
        0x000A,
        -1
    },
    {
        "Write_Context",
        0x000B,
        -1
    },
    {
        "SW_PC",
        0x000C,
        -1
    },
    {
        "Immed_Branch_Taken",
        0x000D,
        -1
    },
    {
        "Branch_Taken",
        0x000E,
        -1
    },
    {
        "Unalign_Load_or_Store",
        0x000F,
        -1
    },
    {
        "Branch_Miss_ornot_taken",
        0x0010,
        -1
    },
    {
        "Cycle",
        0x0011,
        -1
    },
    {
        "Branch_Spec",
        0x0012,
        -1
    },
    {
        "Mem_Access",
        0x0013,
        -1
    },
    {
        "L1I_Cache_Access",
        0x0014,
        -1
    },
    {
        "L1D_Cache_WB",
        0x0015,
        -1
    },
    {
        "L2D_Cache_Access",
        0x0016,
        -1
    },
    {
        "L2D_Cache_Refill",
        0x0017,
        -1
    },
    {
        "L2D_Cache_WB",
        0x0018,
        -1
    },
    {
        "Bus_Access",
        0x0019,
        -1
    },
    {
        "Memory_Error",
        0x001A,
        -1
    },
    {
        "Spec_Exec",
        0x001B,
        -1
    },
    {
        "TTBR",
        0x001C,
        -1
    },
    {
        "Bus_Cycle",
        0x001D,
        -1
    },
    {
        "Chain",
        0x001E,
        -1
    },
    {
        "L1D_Cache_Alloc_WO_Refill",
        0x001F,
        -1
    },
    {
        "L2D_Cache_Alloc_WO_Refill",
        0x0020,
        -1
    },
    {
        "Branch",
        0x0021,
        -1
    },
    {
        "Branch_Miss",
        0x0022,
        -1
    },
    {
        "NOP_frontend",
        0x0023,
        -1
    },
    {
        "NOP_backend",
        0x0024,
        -1
    },
    {
        "L1DTLB_Access",
        0x0025,
        -1
    },
    {
        "L1I_TLB_Acess",
        0x0026,
        -1
    },
    {
        "L2I_Cache_Access",
        0x0027,
        -1
    },
    {
        "L2I_Cache_Refill",
        0x0028,
        -1
    },
    {
        "L3D_Cache_Alloc_WO_Refill",
        0x0029,
        -1
    },
    {
        "L3D_Cache_Refill",
        0x002A,
        -1
    },
    {
        "L3D_Cache_Access",
        0x002B,
        -1
    },
    {
        "L3D_Cache_WB",
        0x002C,
        -1
    },
    {
        "L2D_TLB_Refill",
        0x002D,
        -1
    },
    {
        "L2I_TLB_Refill",
        0x002E,
        -1
    },
    {
        "L2D_TLB_Access",
        0x002F,
        -1
    },
    {
        "L2I_TLB_Access",
        0x0030,
        -1
    },
    {
        "Remote_Access",
        0x0031,
        -1
    },
    {
        "LL_Cache_Access",
        0x0032,
        -1
    },
    {
        "LL_Cache_Miss",
        0x0033,
        -1
    },
    {
        "DTLB_Walk",
        0x0034,
        -1
    },
    {
        "ITLB_Walk",
        0x0035,
        -1
    },
    {
        "LL_Cache_RD",
        0x0036,
        -1
    },
    {
        "LL_Cache_RD_Miss",
        0x0037,
        -1
    },
    {
        "Remote_Access_RD",
        0x0038,
        -1
    },
    {
        "L1D_Cache_Long_Miss",
        0x0039,
        -1
    },
    {
        "OP_Arch_Exec",
        0x003A,
        -1
    },
    {
        "OP_Spec_Exec",
        0x003B,
        -1
    },
    {
        "Stall",
        0x003C,
        -1
    },
    {
        "Stall_Slot_Frontend",
        0x003D,
        -1
    },
    {
        "Stall_Slot_Backend",
        0x003E,
        -1
    },
    {
        "Stall_Slot",
        0x003F,
        -1
    },
    {
        "L1D_Cache_RD",
        0x0040,
        -1
    },
    {
        "L1D_Cache_WR",
        0x0041,
        -1
    },
    {
        "L1D_Cache_Refill_RD",
        0x0042,
        -1
    },
    {
        "L1D_Cache_Refill_WR",
        0x0043,
        -1
    },
    {
        "L1D_Cache_Refill_Inner",
        0x0044,
        -1
    },
    {
        "L1D_Cache_Refill_Outer",
        0x0045,
        -1
    },
    {
        "L1D_Cache_WB_V",
        0x0046,
        -1
    },
    {
        "L1D_Cache_WB_C",
        0x0047,
        -1
    },
    {
        "L1D_Cache_INV",
        0x0048,
        -1
    },
    {
        "",
        0x0049,
        -1
    },
    {
        "",
        0x004A,
        -1
    },
    {
        "",
        0x004B,
        -1
    },
    {
        "L1D_TLB_Refill_RD",
        0x004C,
        -1
    },
    {
        "L1D_TLB_Refill_WR",
        0x004D,
        -1
    },
    {
        "L1D_TLB_Access_RD",
        0x004E,
        -1
    },
    {
        "L1D_TLB_Access_WR",
        0x004F,
        -1
    },
    {
        "L2D_Cache_Access_RD",
        0x0050,
        -1
    },
    {
        "L2D_Cache_Access_WR",
        0x0051,
        -1
    },

    {
        "L2D_Cache_Refill_RD",
        0x0052,
        -1
    },
    {
        "L2D_Cache_Refill_WR",
        0x0053,
        -1
    },
    {
        "",
        0x0054,
        -1
    },
    {
        "",
        0x0055,
        -1
    },
    {
        "L2D_Cache_WB_V",
        0x0056,
        -1
    },
    {
        "L2D_Cache_WB_C",
        0x0057,
        -1
    },

    {
        "L2D_Cache_WB_INV",
        0x0058,
        -1
    },
    {
        "",
        0x0059,
        -1
    },
    {
        "",
        0x005A,
        -1
    },
    {
        "",
        0x005B,
        -1
    },
    {
        "L2D_TLB_Refill_RD",
        0x005C,
        -1
    },
    {
        "L2D_TLB_Refill_WR",
        0x005D,
        -1
    },
    {
        "L2D_TLB_Access_RD",
        0x005E,
        -1
    },
    {
        "L2D_TLB_Acess_WR",
        0x005F,
        -1
    },
    {
        "Bus_Access_RD",
        0x0060,
        -1
    },
    {
        "Bus_Access_WR",
        0x0061,
        -1
    },
    {
        "Bus_Access_Shared",
        0x0062,
        -1
    },
    {
        "Bus_Access_NotShared",
        0x0063,
        -1
    },
    {
        "Bus_Access_Normal",
        0x0064,
        -1
    },
    {
        "Bus_Access_Periph",
        0x0065,
        -1
    },
    {
        "Mem_Access_RD",
        0x0066,
        -1
    },
    {
        "Mem_Access_WR",
        0x0067,
        -1
    },
    {
        "Load_Spec",
        0x0070,
        -1
    },
    {
        "Store_Spec",
        0x0071,
        -1
    },
    {
        "Load_Store_Spec",
        0x0072,
        -1
    },
    {
        "Int_Proc_Spec",
        0x0073,
        -1
    },
    {
        "SW_PC_Spec",
        0x0076,
        -1
    },
    {
        "Immed_Branch_Spec",
        0x0078,
        -1
    },
    {
        "Proc_Ret_Spec",
        0x0079,
        -1
    },
    {
        "Ind_Branch_Spec",
        0x007A,
        -1
    },
    {
        "ISB_Spec",
        0x007C,
        -1
    },
    {
        "DSB_Spec",
        0x007D,
        -1
    },
    {
        "DMB_Spec",
        0x007E,
        -1
    },
    {
        "Exception_Other",
        0x0081,
        -1
    },
    {
        "Exception_SVC",
        0x0082,
        -1
    },
    {
        "Exception_I_Abort",
        0x0083,
        -1
    },
    {
        "Exception_D_Abort",
        0x0084,
        -1
    },
    {
        "Exception_IRQ",
        0x0086,
        -1
    },
    {
        "Exception_FIQ",
        0x0087,
        -1
    },
    {
        "Exception_SMC",
        0x0088,
        -1
    },
    {
        "Exception_HVC",
        0x008A,
        -1
    },
    {
        "Exception_I_Trap",
        0x008B,
        -1
    },
    {
        "Exception_D_Abort",
        0x008C,
        -1
    },
    {
        "Exception_Other_Trap",
        0x008D,
        -1
    },
    {
        "Exception_Trap_IRQ",
        0x008E,
        -1
    },
    {
        "Exception_Trap_FIQ",
        0x008F,
        -1
    },
    {
        "L3D_Cache_Access_RD",
        0x00A0,
        -1
    },
    {
        "L3D_Cache_Access_WR",
        0x00A1,
        -1
    },
    {
        "L3D_Cache_Refill_RD",
        0x00A2,
        -1
    },
    {
        "L3D_Cache_Refill_WR",
        0x00A3,
        -1
    },
    {
        "L3D_Cache_WB_V",
        0x00A6,
        -1
    },
    {
        "L3D_Cache_WB_C",
        0x00A7,
        -1
    },
    {
        "L3D_Cache_INV",
        0x00A8,
        -1
    },
    {
        "Constant_Freq_Cycle",
        0x4004,
        -1
    },
    {
        "Mem_Stall_Cycle",
        0x4005,
        -1
    },
    {
        "L1I_Access_Long_Miss",
        0x4006,
        -1
    },
    {
        "L2D_Cache_Long_Miss_Rd",
        0x4009,
        -1
    },
    {
        "L2I_Cache_Long_Miss",
        0x400A,
        -1
    },
    {
        "L2D_Cache_Long_Miss_Rd",
        0x400B,
        -1
    },
    {
        "TRB_Wrap",
        0x400C,
        -1
    },
    {
        "PMU_OVERFLOW_EL0",
        0x400D,
        -1
    },
    {
        "TRB_Trig",
        0x400E,
        -1
    },
    {
        "PMU_OVERFLOW_EL2",
        0x400F,
        -1
    },
    {
        "Prefetch_Spec",
        0x8087,
        -1
    },
    {
        "Immed_Branch_Taken",
        0x8108,
        -1
    },
    {
        "Immed_Branch_not_Taken",
        0x8109,
        -1
    },
    {
        "Ind_Branch_Taken",
        0x810A,
        -1
    },
    {
        "Ind_Branch_not_Taken",
        0x810B,
        -1
    },
    {
        "Ind_Branch_noproc_Taken",
        0x810C,
        -1
    },
    {
        "Ind_Branch_noproc_not_Taken",
        0x810D,
        -1
    },
    {
        "Proc_Return_Taken",
        0x810E,
        -1
    },
    {
        "Proc_Return_not_Taken",
        0x810F,
        -1
    },
    {
        "Immed_Branch_Pred",
        0x8110,
        -1
    },
    {
        "Immed_Branch_Miss",
        0x8111,
        -1
    },
    {
        "Ind_Branch_Pred",
        0x8112,
        -1
    },
    {
        "Ind_Branch_Miss",
        0x8113,
        -1
    },
    {
        "Proc_Pred",
        0x8114,
        -1
    },
    {
        "Proc_Miss",
        0x8115,
        -1
    },
    {
        "Ind_noproc_Pred",
        0x8116,
        -1
    },
    {
        "Ind_noproc_Miss",
        0x8117,
        -1
    },
    {
        "Branch_Pred_Taken",
        0x8118,
        -1
    },
    {
        "Branch_Miss_Taken",
        0x8119,
        -1
    },
    {
        "Branch_Pred_not_Taken",
        0x811A,
        -1
    },
    {
        "Branch_Miss_not_Taken",
        0x811B,
        -1
    },
    {
        "Branch_Pred",
        0x811C,
        -1
    },
    {
        "Ind_Branch",
        0x811D,
        -1
    },
    {
        "Ind_Branch_noproc",
        0x811E,
        -1
    },
    {
        "BRB_Filtrate",
        0x811F,
        -1
    },
    {
        "Inst_Fetch_PerCycle",
        0x8120,
        -1
    },
    {
        "Mem_RD_PerCycle",
        0x8121,
        -1
    },
    {
        "Inst_Fetch",
        0x8124,
        -1
    },
    {
        "Ind_Branch_noproc_nolink_Taken",
        0x8179,
        -1
    },
    {
        "Branch_link_Taken",
        0x817A,
        -1
    },
    {
        "Branch_nolink_Taken",
        0x817B,
        -1
    },
    {
        "Ind_Branch_link_Taken",
        0x817C,
        -1
    },
    {
        "Ind_Branch_nolink_Taken",
        0x817D,
        -1
    },
    {
        "Dir_Branch_link_Taken",
        0x817E,
        -1
    },
    {
        "Dir_Branch_nolink_Taken",
        0x817F,
        -1
    },
    {
        "Uncond_Branch",
        0x8180,
        -1
    },
    {
        "Cond_Branch",
        0x8181,
        -1
    },
    {
        "Cond_Branch_Taken",
        0x8182,
        -1
    },
    {
        "Hint_Cond_Branch",
        0x8183,
        -1
    },
    {
        "Load_Any",
        0x82A8,
        -1
    },
    {
        "Store_Any",
        0x82A9,
        -1
    },
    {
        "Load_or_Store_Any",
        0x82AA,
        -1
    },
    {
        "DSP",
        0x82AB,
        -1
    },
};