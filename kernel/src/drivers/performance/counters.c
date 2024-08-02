#include <drivers/performance/counters.h>
#include <drivers/performance/pmu_constants.h>

#include <stdlib/types.h>

int active_counters[MAX_COUNTERS];
uint64_t counter_final[MAX_COUNTERS+1];
uint64_t counter_start[MAX_COUNTERS+1];

int n_active_counters;

void init_pmu(void)
{
    for (int i = 0; i < MAX_COUNTERS; i++) {
        active_counters[i] = 0;
        counter_start[i] = 0;
        counter_final[i] = 0;
    }

    int t1 = read_perf_register(PMCR_EL0);
    write_perf_register(PMCR_EL0, t1 | PMCR_EL0_LC_BIT);

    counter_start[MAX_COUNTERS] = 0;
    counter_final[MAX_COUNTERS] = 0;

    n_active_counters = 0;
}

uint64_t deinit_pmu(void)
{
    uint64_t overflow = read_perf_register(PMOVSCLR_EL0);
    return overflow;
}

void enable_counter(int val)
{
    int type;

    active_counters[n_active_counters] = val;

    write_perf_register(PMSELR_EL0, n_active_counters);
    write_perf_register(PMXEVTYPER_EL0, 0);
    write_perf_register(PMSELR_EL0, n_active_counters);

    type = read_perf_register(PMXEVTYPER_EL0);
    write_perf_register(PMXEVTYPER_EL0,
        (type & ~PMEVTYPERX_EL0_EVT_MASK) | (val));
    
    int t1 = read_perf_register(PMCR_EL0);
    write_perf_register(PMCR_EL0, t1 | PMCR_EL0_C_BIT);
    t1 = read_perf_register(PMCR_EL0);
    write_perf_register(PMCR_EL0, t1 | PMCR_EL0_P_BIT);

    int t2 = read_perf_register(PMCNTENSET_EL0);
    write_perf_register(PMCNTENSET_EL0, t2 | (1ULL << n_active_counters));

    n_active_counters++;
}

void disable_counter(int idx)
{
    int t1 = read_perf_register(PMCNTENCLR_EL0);
    write_perf_register(PMCNTENCLR_EL0, t1 | (1ULL << idx));
}

void enable_cycle_counter(void)
{
    int type;

    write_perf_register(PMSELR_EL0, MAX_COUNTERS);
    write_perf_register(PMXEVTYPER_EL0, 0);
    write_perf_register(PMSELR_EL0, MAX_COUNTERS);
    type = read_perf_register(PMXEVTYPER_EL0);
    write_perf_register(PMXEVTYPER_EL0,
        (type & ~PMEVTYPERX_EL0_EVT_MASK) | (0xC0BE));

    int t1 = read_perf_register(PMCR_EL0);
    write_perf_register(PMCR_EL0, t1 | PMCR_EL0_C_BIT);
    t1 = read_perf_register(PMCR_EL0);
    write_perf_register(PMCR_EL0, t1 | PMCR_EL0_P_BIT);

    int t2 = read_perf_register(PMCNTENSET_EL0);
    write_perf_register(PMCNTENSET_EL0, t2 | (1ULL << MAX_COUNTERS));
}

void disable_cycle_counter(void)
{
    int t1 = read_perf_register(PMCNTENCLR_EL0);
    write_perf_register(PMCNTENCLR_EL0, t1 | (1ULL << MAX_COUNTERS));
}

void profile_start(void)
{
    for (int i = 0; i < n_active_counters; i++) {
        write_perf_register(PMSELR_EL0, i);
        counter_start[i] = read_perf_register(PMXEVCNTR_EL0);
    }

    // Read cycle counter
    counter_start[MAX_COUNTERS] = read_perf_register(PMCCNTR_EL0);
}

void profile_end(void)
{
    for (int i = 0; i < n_active_counters; i++) {
        write_perf_register(PMSELR_EL0, i);
        counter_final[i] = read_perf_register(PMXEVCNTR_EL0);
    }

    // Read cycle counter
    counter_final[MAX_COUNTERS] = read_perf_register(PMCCNTR_EL0);
}

void enable_global_counters(void)
{
    int t1 = read_perf_register(PMCR_EL0);
    write_perf_register(PMCR_EL0, t1 | PMCR_EL0_E_BIT);
}

void disable_global_counters(void)
{
    int t1 = read_perf_register(PMCR_EL0);
    write_perf_register(PMCR_EL0, t1 & ~PMCR_EL0_E_BIT);
}

// General process for profiling code
// Choose which events (events.h)
// 
// Call init_pmu() function
// Call enable_counter() for each event
// Call enable_cycle_counter()
// Call enable_global_counters()
// Call profile_start()
// 
// *** Do code to profile ***
// 
// Call disable_global_counters()
// Call disable_counter() for each event
// Call disable_cycle_counter()
// Call profile_end()
// Call deinit_pmu()
