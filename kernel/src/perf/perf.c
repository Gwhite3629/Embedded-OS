#include <perf/perf.h>
#include <drivers/performance/events.h>
#include <drivers/performance/counters.h>
#include <stdlib/printk.h>
#include <stdlib/serial.h>

int events[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

char *names[10];

int n_events;

void init_events(void)
{
    int i = 0;
    n_events = 0;

    for(i = 0; i < 180; i++) {
        perf_events[i].hash = perf_hash(perf_events[i].name);
    }

    qsort(perf_events, 180, sizeof(perf_event_t), &perf_cmp);
}

int get_events(void)
{
    int ret = 0;
    uint32_t buflen = 0;
	char buf[4096];
    char n_chosen;
    char ch;
    bool done = 0;
    perf_event_t *user_event;
    // Get user inquiry
    memset(buf, '\0', 4096);
    printk("\nEnter the number of desired events (1-9)\n> ");

    n_chosen = uart_getc();
    uart_write(&n_chosen, 1);
    uart_write("\n", 1);

    int chosen = (int)(n_chosen - 48);
    if ((chosen < 1) | (chosen > 9)) {
        printk("Number not valid\n");
        return -1;
    }
    n_events = chosen;

    printk("Enter the events one at a time, pressing enter at the end of each\n");

    for (int i = 0; i < chosen; i++) {
        while(done == 0) {
            switch(ch = uart_getc()) {
                // Backspace or delete
                case(0x8):
                case(0x7f):
                    if (buflen > 0) {
                        buflen--;
                        buf[buflen] = '\0';
                    }
                    break;
                // Newline or carriage return
                case(0xa):
                case(0xd):
                    done = 1;
                    break;
                // Any non-special character
                default:
                    buf[buflen] = ch;
                    buflen++;
            }
            // Display character to screen
            ret = uart_write(&ch, 1);
            if (ret < 0) {
                printk("\nERROR: %d", ret);
            }
        }

        user_event = perf_lookup(buf, perf_events, 180);
        if (user_event == NULL) {
            printk("Invalid event, try again\n");
            i--;
        } else {
            events[i] = user_event->event_number;
            printk("Allocing\n");
            new(names[i], buflen+1, char);
            memset(names[i], '\0', buflen+1);
            printk("Alloced\n");
            strncpy(names[i], buf, buflen);
            printk("Chosen Event: %s with number %d\n", names[i], events[i]);
        }

        memset(buf, '\0', 4096);
        buflen = 0;
        done = 0;
    }

exit:
    return ret;
}

// Choose which events (events.h)
// Call init_pmu() function
// Call profile_start()
// Call enable_cycle_counter()
// Call enable_counter() for each event
// Call enable_cycle_counter()
void begin_profiling(void)
{
    int ret = 0;
    int i = 0;
    ret = get_events();
    if (ret < 0) {
        printk("Failed to perform perf\n");
        return;
    }
    init_pmu();
    while(events[i] != -1) {
        enable_counter(events[i]);
        i++;
    }
    enable_cycle_counter();
    profile_start();
    enable_global_counters();
}

// Call disable_global_counters()
// Call disable_counter() for each event
// Call disable_cycle_counter()
// Call profile_end()
// Call deinit_pmu()
uint64_t end_profiling(void)
{
    int i = 0;
    disable_global_counters();
    profile_end();
    while(events[i] != -1) {
        disable_counter(events[i]);
        i++;
    }
    disable_cycle_counter();
    uint64_t overflow = deinit_pmu();
    return overflow;
}

int perf_list(char *buf)
{
    for (int i = 0; i < 180; i++) {
        printk("%s\n", perf_events[i].name);
    }

    return 0;
}

void perf_print(void)
{
    int i = 0;

    printk("TIME: %lu ticks @ %lu Hz\n", time_end - time_start, timer_freq);

    for (i = 0; i < n_events; i++) {
        printk("\n%s\n", names[i]);
        printk("\tINITIAL: %d\n", counter_start[i]);
        printk("\tFINAL:   %d\n", counter_final[i]);
        printk("\tDELTA:   %d\n", counter_final[i] - counter_start[i]);
    }

    printk("\nCycles\n");
    printk("\tINITIAL: %d\n", counter_start[MAX_COUNTERS]);
    printk("\tFINAL:   %d\n", counter_final[MAX_COUNTERS]);
    printk("\tDELTA:   %d\n", counter_final[MAX_COUNTERS] - counter_start[MAX_COUNTERS]);
}

int perf_cleanup(void)
{
    int ret = 0;
    for (int i = 0; i < n_events; i++) {
        events[i] = -1;
        del(names[i]);
    }
exit:
    return ret;
}
