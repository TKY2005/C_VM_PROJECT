#include<VM/vm_interrupts.h>

#include<pthread.h>
#include<stdio.h>
#include<stdint.h>

static pthread_mutex_t interrupt_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static int ic_lock(void) {
    int result = pthread_mutex_lock(&interrupt_queue_mutex);
    if (result != 0) {
        fprintf(stderr, "Unable to lock interrupt queue (error %d).\n", result);
        return -1;
    }
    return 0;
}

static void ic_unlock(void) {
    int result = pthread_mutex_unlock(&interrupt_queue_mutex);
    if (result != 0) {
        fprintf(stderr, "Unable to unlock interrupt queue (error %d).\n", result);
    }
}

interrupt_controller ic_init() {
    interrupt_controller ic = {0};
    ic.interrupt_queue = queue_init();
    return ic;
}

int ic_interrupt_enqueue(interrupt_controller* ic, uint8_t icode) {
    if (ic_lock() < 0) return -1;
    queue_enqueue_back(ic->interrupt_queue, (void*) (uintptr_t) icode);
    ic_unlock();
    return 0;
}

int ic_interrupt_line_status(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    int pending = !queue_empty(ic->interrupt_queue);
    ic_unlock();
    return pending;
}

int ic_interrupt_acknowledge(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    int icode = (int) (uintptr_t) queue_dequeue_front(ic->interrupt_queue);
    ic_unlock();
    return icode;
}

void ic_reset(interrupt_controller* ic) {
    if (ic_lock() < 0) return;
    queue_free(ic->interrupt_queue);
    ic->interrupt_queue = queue_init();
    ic_unlock();
}

void ic_free(interrupt_controller* ic) {
    if (ic_lock() < 0) return;
    queue_free(ic->interrupt_queue);
    ic->interrupt_queue = NULL;
    ic_unlock();
}