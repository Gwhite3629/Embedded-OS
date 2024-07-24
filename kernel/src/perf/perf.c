#include <perf/perf.h>
#include <drivers/performance/events.h>

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
    perf_event_t user_event
    // Get user inquiry
    printf("Enter the number of desired events (1-9)\n> ");

    n_chosen = serial_read();
    serial_write(&n_chosen, 1);
    serial_write("\n", 1);

    int chosen = (int)n_chosen - 30;
    if ((chosen < 1) | (chosen > 9)) {
        printf("Number not valid\n");
        return;
    }
    n_events = chosen;

    printf("Enter the events one at a time, pressing enter at the end of each\n")

    for (int i = 0; i < chosen; i++) {
        switch(ch = serial_read()) {
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
                break;
            // Any non-special character
            default:
                buf[buflen] = ch;
                buflen++;
        }
        // Display character to screen
        ret = putchar(ch);
        if (ret != 0) {
             printf("\nERROR: %d", ret);
        }

        user_event = perf_lookup(buf, perf_events, 180);
        if (user_event == NULL) {
            printf("Invalid event, try again\n");
            i--;
        } else {
            events[i] = user_event.event_number;
            new(names[i], buflen, char *);
            strncpy(names[i], buf, buflen);
            printf("\n");
        }

        memset(buf, '\0', 4096);
        buflen = 0;
    }

exit:
    return ret;
}

// Choose which events (events.h)
// Call init_pmu() function
// Call enable_counter() for each event
// Call enable_cycle_counter()
// Call enable_global_counters()
// Call profile_start()
void begin_profiling(void)
{
    int i = 0;
    get_events();
    init_pmu();
    while(events[i] != -1) {
        enable_counter(events[i]);
        i++;
    }
    enable_cycle_counter();
    enable_global_counters();
    profile_start();
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
    while(events[i] != -1) {
        disable_counter(events[i]);
        i++;
    }
    disable_cycle_counter();
    profile_end();
    uint64_t overflow = deinit_pmu();
}

int perf_list(const char *buf)
{
    for (int i = 0; i < n_events; i++) {
        printf("%s\n", perf_events[i].name);
    }

    return 0;
}

void perf_print(void)
{
    int i = 0;

    for (i = 0; i < n_events; i++) {
        printf("\n%s\n", names[i]);
        printf("\tINITIAL: %d\n", counter_start[i]);
        printf("\tFINAL:   %d\n", counter_final[i]);
        printf("\tDELTA:   %d\n", counter_final[i] - counter_start[i]);
    }
}

int perf_cleanup(void)
{
    int ret = 0;
    for (i = 0; i < n_events; i++) {
        events[i] = -1;
        del(names[i]);
    }
exit:
    return ret;
}