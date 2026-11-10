#ifndef CLOCK_H
#define CLOCK_H

#include<stdint.h>
#include<pthread.h>

#define TIMER_INTERRUPT_VECTOR 0x20

void clock_configure(uint64_t frequency_hz);
void clock_reset(void);
void clock_tick(void);
void clock_shutdown(void);
void timer_sleep(uint64_t ms);

void* timer_tick_task(void* arg);

pthread_t* clock_timer_start(uint64_t interval);
void clock_timer_stop(pthread_t* timer_thread);

#endif
