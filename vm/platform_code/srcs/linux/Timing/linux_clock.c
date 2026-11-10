#include<Timing/clock.h>

#include<errno.h>
#include<stdint.h>
#include<time.h>

#define CLOCK_NANOSECONDS_PER_SECOND UINT64_C(1000000000)
#define CLOCK_SPIN_GUARD_NS UINT64_C(50000)

static uint64_t clock_frequency_hz = 0;
static uint64_t clock_fractional_remainder = 0;
static struct timespec clock_deadline = {0};
static int clock_initialized = 0;

static void clock_add_nanoseconds(struct timespec* time, uint64_t nanoseconds) {
    time->tv_sec += (time_t)(nanoseconds / CLOCK_NANOSECONDS_PER_SECOND);
    nanoseconds %= CLOCK_NANOSECONDS_PER_SECOND;

    uint64_t new_nanoseconds = (uint64_t)time->tv_nsec + nanoseconds;
    if (new_nanoseconds >= CLOCK_NANOSECONDS_PER_SECOND) {
        time->tv_sec++;
        new_nanoseconds -= CLOCK_NANOSECONDS_PER_SECOND;
    }
    time->tv_nsec = (long)new_nanoseconds;
}

static void clock_subtract_nanoseconds(struct timespec* time, uint64_t nanoseconds) {
    uint64_t seconds = nanoseconds / CLOCK_NANOSECONDS_PER_SECOND;
    uint64_t remaining_nanoseconds = nanoseconds % CLOCK_NANOSECONDS_PER_SECOND;

    time->tv_sec -= (time_t)seconds;
    if ((uint64_t)time->tv_nsec < remaining_nanoseconds) {
        time->tv_sec--;
        time->tv_nsec = (long)(CLOCK_NANOSECONDS_PER_SECOND + (uint64_t)time->tv_nsec - remaining_nanoseconds);
    } else {
        time->tv_nsec -= (long)remaining_nanoseconds;
    }
}

static int clock_timespec_is_before(const struct timespec* left, const struct timespec* right) {
    return left->tv_sec < right->tv_sec ||
        (left->tv_sec == right->tv_sec && left->tv_nsec < right->tv_nsec);
}

void clock_configure(uint64_t frequency_hz) {
    clock_frequency_hz = frequency_hz;
    clock_reset();
}

void clock_reset(void) {
    clock_fractional_remainder = 0;
    clock_initialized = clock_gettime(CLOCK_MONOTONIC, &clock_deadline) == 0;
}

void clock_tick(void) {
    if (clock_frequency_hz == 0) return;
    if (!clock_initialized) clock_reset();
    if (!clock_initialized) return;

    uint64_t whole_nanoseconds = CLOCK_NANOSECONDS_PER_SECOND / clock_frequency_hz;
    uint64_t fractional_nanoseconds = CLOCK_NANOSECONDS_PER_SECOND % clock_frequency_hz;

    clock_add_nanoseconds(&clock_deadline, whole_nanoseconds);
    clock_fractional_remainder += fractional_nanoseconds;
    if (clock_fractional_remainder >= clock_frequency_hz) {
        clock_add_nanoseconds(&clock_deadline, 1);
        clock_fractional_remainder -= clock_frequency_hz;
    }

    for (;;) {
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now) != 0 ||
            !clock_timespec_is_before(&now, &clock_deadline)) {
            return;
        }

        struct timespec wait_until = clock_deadline;
        uint64_t remaining_nanoseconds =
            (uint64_t)(clock_deadline.tv_sec - now.tv_sec) * CLOCK_NANOSECONDS_PER_SECOND;
        if (clock_deadline.tv_nsec >= now.tv_nsec) {
            remaining_nanoseconds += (uint64_t)(clock_deadline.tv_nsec - now.tv_nsec);
        } else {
            remaining_nanoseconds -= (uint64_t)(now.tv_nsec - clock_deadline.tv_nsec);
        }

        if (remaining_nanoseconds > CLOCK_SPIN_GUARD_NS) {
            clock_subtract_nanoseconds(&wait_until, CLOCK_SPIN_GUARD_NS);
            int result = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wait_until, NULL);
            if (result != 0 && result != EINTR) return;
        }
    }
}

void clock_shutdown(void) {
    clock_frequency_hz = 0;
    clock_fractional_remainder = 0;
    clock_initialized = 0;
}
