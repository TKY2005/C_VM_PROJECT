#include<Timing/clock.h>

#include<stdint.h>
#include<unistd.h>

void clock_simulate_delay(uint64_t ms) {
    usleep(ms * 1000);
}