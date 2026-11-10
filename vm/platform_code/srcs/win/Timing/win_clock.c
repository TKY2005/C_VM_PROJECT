#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include<Timing/clock.h>

#include<stdint.h>
#include<windef.h>
#include<winnt.h>
#include<handleapi.h>
#include<profileapi.h>
#include<synchapi.h>

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

#define CLOCK_COUNTER_UNITS_PER_SECOND UINT64_C(10000000)
#define CLOCK_SPIN_GUARD_UNITS UINT64_C(500)

static uint64_t clock_frequency_hz = 0;
static uint64_t clock_fractional_remainder = 0;
static uint64_t clock_deadline = 0;
static uint64_t counter_frequency = 0;
static HANDLE clock_waitable_timer = NULL;
static int clock_initialized = 0;

void clock_configure(uint64_t frequency_hz) {
    if (clock_waitable_timer != NULL) {
        CloseHandle(clock_waitable_timer);
        clock_waitable_timer = NULL;
    }

    clock_frequency_hz = frequency_hz;
    clock_fractional_remainder = 0;
    clock_initialized = 0;

    LARGE_INTEGER performance_frequency;
    if (!QueryPerformanceFrequency(&performance_frequency) || performance_frequency.QuadPart <= 0) {
        clock_frequency_hz = 0;
        return;
    }
    counter_frequency = (uint64_t)performance_frequency.QuadPart;

    clock_waitable_timer = CreateWaitableTimerExW(
        NULL,
        NULL,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
        TIMER_MODIFY_STATE | SYNCHRONIZE
    );
    if (clock_waitable_timer == NULL) {
        clock_waitable_timer = CreateWaitableTimerExW(
            NULL,
            NULL,
            0,
            TIMER_MODIFY_STATE | SYNCHRONIZE
        );
    }

    clock_reset();
}

void clock_reset(void) {
    clock_fractional_remainder = 0;
    LARGE_INTEGER now;
    clock_initialized = QueryPerformanceCounter(&now) != 0;
    if (clock_initialized) clock_deadline = (uint64_t)now.QuadPart;
}

void clock_tick(void) {
    if (clock_frequency_hz == 0) return;
    if (!clock_initialized) clock_reset();
    if (!clock_initialized) return;

    uint64_t whole_counter_units = counter_frequency / clock_frequency_hz;
    uint64_t fractional_counter_units = counter_frequency % clock_frequency_hz;

    clock_deadline += whole_counter_units;
    clock_fractional_remainder += fractional_counter_units;
    if (clock_fractional_remainder >= clock_frequency_hz) {
        clock_deadline++;
        clock_fractional_remainder -= clock_frequency_hz;
    }

    for (;;) {
        LARGE_INTEGER now;
        if (!QueryPerformanceCounter(&now) || (uint64_t)now.QuadPart >= clock_deadline) return;

        uint64_t remaining_counter_units = clock_deadline - (uint64_t)now.QuadPart;
        if (remaining_counter_units > CLOCK_SPIN_GUARD_UNITS && clock_waitable_timer != NULL) {
            uint64_t wait_counter_units = remaining_counter_units - CLOCK_SPIN_GUARD_UNITS;
            uint64_t wait_100ns =
                (wait_counter_units * CLOCK_COUNTER_UNITS_PER_SECOND + counter_frequency - 1) /
                counter_frequency;
            if (wait_100ns == 0) wait_100ns = 1;

            LARGE_INTEGER due_time;
            due_time.QuadPart = -(LONGLONG)wait_100ns;
            if (SetWaitableTimer(clock_waitable_timer, &due_time, 0, NULL, NULL, FALSE)) {
                WaitForSingleObject(clock_waitable_timer, UINT32_MAX);
            } else {
                YieldProcessor();
            }
        } else {
            YieldProcessor();
        }
    }
}

void clock_shutdown(void) {
    if (clock_waitable_timer != NULL) {
        CloseHandle(clock_waitable_timer);
        clock_waitable_timer = NULL;
    }
    clock_frequency_hz = 0;
    clock_fractional_remainder = 0;
    clock_initialized = 0;
}
