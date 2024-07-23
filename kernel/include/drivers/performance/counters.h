#ifndef _COUNTERS_H_
#define _COUNTERS_H_

#include "pmu_constants.h"
#include <stdlib/types.h>

#define MAX_COUNTERS 31
// Total number of counters
// Index number is used for reading and counter number is stored at index
int active_counters[MAX_COUNTERS];
uint64_t counter_final[MAX_COUNTERS+1];
uint64_t counter_start[MAX_COUNTERS+1];

int n_active_counters;

static int read_perf_register(int reg)
{
    int v;

    switch (reg) {
        case PMCR_EL0:
            asm volatile ("mrs %0, pmcr_el0" : "=r" (v));
            break;
        case PMCNTENSET_EL0:
            asm volatile ("mrs %0, pmcntenset_el0" : "=r" (v));
            break;
        case PMCNTENCLR_EL0:
            asm volatile ("mrs %0, pmcntenclr_el0" : "=r" (v));
            break;
        case PMSELR_EL0:
            asm volatile ("mrs %0, pmselr_el0" : "=r" (v));
            break;
        case PMCCNTR_EL0:
            asm volatile ("mrs %0, pmccntr_el0" : "=r" (v));
            break;
        case PMXEVTYPER_EL0:
            asm volatile ("mrs %0, pmxevtyper_el0" : "=r" (v));
            break;
        case PMCCFILTR_EL0:
            asm volatile ("mrs %0, pmccfiltr_el0" : "=r" (v));
            break;
        case PMXEVCNTR_EL0:
            asm volatile ("mrs %0, pmxevcntr_el0" : "=r" (v));
            break;
        case PMOVSCLR_EL0:
            asm volatile ("mrs %0, pmovsclr_el0" : "=r" (v));
            break;
        default:
            // Error no register
            break;
    }

    return v;
}

static void write_perf_register(int reg, int v)
{
    switch (reg) {
        case PMCR_EL0:
            asm volatile ("msr pmcr_el0, %0" : : "r" (v));
            break;
        case PMCNTENSET_EL0:
            asm volatile ("msr pmcntenset_el0, %0" : : "r" (v));
            break;
        case PMCNTENCLR_EL0:
            asm volatile ("msr pmcntenclr_el0, %0" : : "r" (v));
            break;
        case PMSELR_EL0:
            asm volatile ("msr pmselr_el0, %0" : : "r" (v));
            break;
        case PMCCNTR_EL0:
            asm volatile ("msr pmccntr_el0, %0" : : "r" (v));
            break;
        case PMXEVTYPER_EL0:
            asm volatile ("msr pmxevtyper_el0, %0" : : "r" (v));
            break;
        case PMCCFILTR_EL0:
            asm volatile ("msr pmccfiltr_el0, %0" : : "r" (v));
            break;
        case PMXEVCNTR_EL0:
            asm volatile ("msr pmxevcntr_el0, %0" : : "r" (v));
            break;
        case PMOVSCLR_EL0:
            asm volatile ("msr pmovsclr_el0, %0" : : "r" (v));
            break;
        default:
            // Error no register
            break;
    }

    asm volatile("isb");
}

void init_pmu(void);

uint64_t deinit_pmu(void);

void enable_counter(int val);

void disable_counter(int idx);

void enable_cycle_counter(void);

void disable_cycle_counter(void);

void profile_start(void);

void profile_end(void);

void enable_global_counters(void);

void disable_global_counters(void);

#endif // _COUNTERS_H_