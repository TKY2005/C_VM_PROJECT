#include<pthread.h>
#include<stdint.h>
#include<stdatomic.h>
#include<stdio.h>
#include<stdlib.h>
#include<VM/vm_interrupts.h>
#include<VM/vm.h>
#include<VM/vm_settings.h>
#include<CPU/CPU.h>

#include<Timing/clock_internal.h>
#include<Timing/clock.h>

static atomic_bool timer_running = ATOMIC_VAR_INIT(false);

void* timer_tick_task(void* arg) {
    uint64_t ms = *(uint64_t*) arg;
    float speedup = vm_conf.timer_speedup_factor;
    uint64_t cycles = (ms * 1000) * speedup;
    uint64_t last_interrupt_cycle = 0;

    free(arg);

    while (atomic_load(&timer_running)) {
        uint64_t elapsed = CPU_get_cycles(vm_cpu);
        if (!atomic_load(&timer_running)) break;
        if (elapsed % cycles != 0 || elapsed == last_interrupt_cycle) continue;
        if (ic_interrupt_enqueue(&vm_ic, TIMER_INTERRUPT_VECTOR) < 0) {
            fprintf(stderr, "Unable to enqueue timer interrupt.\n");
            atomic_store(&timer_running, false);
            break;
        }
        last_interrupt_cycle = elapsed;
    }
    return NULL;
}

pthread_t* clock_timer_start(uint64_t interval) {
    if (interval == 0) {
        fprintf(stderr, "Timer interval must be greater than zero.\n");
        return NULL;
    }
    if (interval > UINT64_MAX / 1000) {
        fprintf(stderr, "Timer interval is too large.\n");
        return NULL;
    }

    uint64_t* interval_arg = malloc(sizeof(uint64_t));
    if (interval_arg == NULL) {
        fprintf(stderr, "Unable to allocate timer interval.\n");
        return NULL;
    }
    *interval_arg = interval;

    pthread_t* t = malloc(sizeof(pthread_t));
    if (t == NULL) {
        fprintf(stderr, "Unable to allocate timer thread handle.\n");
        free(interval_arg);
        return NULL;
    }

    atomic_store(&timer_running, true);
    int result = pthread_create(t, NULL, timer_tick_task, interval_arg);
    if (result != 0) {
        fprintf(stderr, "Unable to start timer thread (error %d).\n", result);
        atomic_store(&timer_running, false);
        free(interval_arg);
        free(t);
        return NULL;
    }
    return t;
}

void clock_timer_stop(pthread_t* timer_thread) {
    if (timer_thread == NULL) return;

    atomic_store(&timer_running, false);
    int result = pthread_join(*timer_thread, NULL);
    if (result != 0) {
        fprintf(stderr, "Unable to join timer thread (error %d).\n", result);
    }
    free(timer_thread);
}