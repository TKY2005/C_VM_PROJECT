#include<Timing/clock.h>

#include<stdint.h>
#include<synchapi.h>
#include<timeapi.h>

void clock_simulate_delay(uint64_t ms) {
    timeBeginPeriod(1);
    Sleep(ms);
    timeEndPeriod(1);
}