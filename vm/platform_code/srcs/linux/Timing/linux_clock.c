#include<Timing/clock.h>

#include<stdint.h>
#include<unistd.h>
#include<stdio.h>

void clock_simulate_delay(uint64_t ms) {
    fflush(stdout);
    usleep(ms * 1000);
}