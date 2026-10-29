#include<Timing/clock.h>

#include<stdint.h>
#include<windows.h>

void clock_simulate_delay(uint64_t ms) {
    timeBeginPeriod(1);
    Sleep(ms);
    timeEndPeriod(1);
}

void timer_sleep(uint64_t ms) {
    timeBeginPeriod(1);
    Sleep(ms);
    timeBeginPeriod(1);
}