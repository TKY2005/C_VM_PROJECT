#define _POSIX_C_SOURCE 199309L

#include<Timing/clock.h>

#include<stdint.h>
#include<time.h>
#include<stdio.h>

void clock_simulate_delay(uint64_t ms) {
    fflush(stdout);
    struct timespec s;
    s.tv_sec = ms / 1000;
    s.tv_nsec = ms * 1000000;
    nanosleep(&s, NULL);
}