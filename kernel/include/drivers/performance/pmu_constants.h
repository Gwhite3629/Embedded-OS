#ifndef _PMU_CONSTANTS_H_
#define _PMU_CONSTANTS_H_

#define PMCR_EL0        0
#define PMCNTENSET_EL0  1
#define PMCNTENCLR_EL0  2
#define PMSELR_EL0      3
#define PMCCNTR_EL0     4
#define PMXEVTYPER_EL0  5
#define PMCCFILTR_EL0   6
#define PMXEVCNTR_EL0   7
#define PMOVSCLR_EL0    8

/* Per-CPU PMCR_EL0 definitions */
#define PMCR_EL0_RESET_VAL			0x00ULL
#define PMCR_EL0_N_SHIFT			11ULL
#define PMCR_EL0_N_MASK				0x1fULL
#define PMCR_EL0_N_BITS				(PMCR_EL0_N_MASK << PMCR_EL0_N_SHIFT)
#define PMCR_EL0_LC_BIT				(1ULL << 6)
#define PMCR_EL0_C_BIT				(1ULL << 2)
#define PMCR_EL0_P_BIT				(1ULL << 1)
#define PMCR_EL0_E_BIT				(1ULL << 0)

/* PMCCFILTR/PMEVTYPER<n> definitions */
#define PMEVTYPERX_EL0_P_BIT		(1ULL << 31)
#define PMEVTYPERX_EL0_NSH_BIT		(1ULL << 27)
#define PMEVTYPERX_EL0_M_BIT		(1ULL << 26)
#define PMEVTYPERX_EL0_DEFAULT		0x00ULL
#define PMEVTYPERX_EL0_EVT_MASK		0x3ffULL


/* MDCR_EL3 definitions */
#define MDCR_MPMX_BIT				(1ULL << 35)
#define MDCR_MCCD_BIT				(1ULL << 34)
#define MDCR_SCCD_BIT				(1ULL << 23)
#define MDCR_SPME_BIT				(1ULL << 17)

/* MDCR_EL2 definitions */
#define MDCR_EL2_HPME_BIT			(1ULL << 7)
#define MDCR_EL2_HPMN_MASK			0x1fULL

/* CurrentEL definitions */
#define MODE_EL_SHIFT				0x2U
#define MODE_EL_MASK				0x3U
#define MODE_EL_WIDTH				0x2U
#define MODE_EL3					0x3U
#define MODE_EL2					0x2U
#define MODE_EL1					0x1U
#define MODE_EL0					0x0U

#define GET_EL(mode)				(((mode) >> MODE_EL_SHIFT) & MODE_EL_MASK)

#endif _PMU_CONSTANTS_H_