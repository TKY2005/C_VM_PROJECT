#include<VM/vm_interrupts.h>
#include<VM/vm.h>

#include<pthread.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>

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
    ic.enabled = IC_ENABLED;
    return ic;
}

int ic_interrupt_enqueue(interrupt_controller* ic, uint8_t icode) {
    if (ic_lock() < 0) return -1;
    if (ic->enabled == IC_DISABLED){ 
        ic_unlock();
        return 0;
    }

    if (ic_isrejected(ic, icode)){ 
        ic_unlock();
        return 0;
    }
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

int ic_cmd_disable(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    ic->enabled = IC_DISABLED;
    ic_unlock();
    return 0;
}

int ic_cmd_enable(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    ic->enabled = IC_ENABLED;
    ic_unlock();
    return 0;
}

int ic_cmd_dropall(interrupt_controller* ic) {
    ic_reset(ic);
    return 0;
}

int ic_cmd_discard_vector(interrupt_controller* ic) {
    uint8_t target = ic->data;
    for(int i = 0; i < queue_size(ic->interrupt_queue); i++) {
        uint8_t current = (uint8_t) (uintptr_t) queue_peek_at(ic->interrupt_queue, i);
        if (current == target) queue_dequeue_at(ic->interrupt_queue, i);
    }
    return 0;
}

int ic_cmd_reject_vector(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    uint8_t vec = ic->data;
    ic->rejection_list[vec] = 1;
    ic_unlock();
    return 0;
}

int ic_cmd_allow_vector(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    uint8_t vec = ic->data;
    ic->rejection_list[vec] = 0;
    ic_unlock();
    return 0;
}

int ic_cmd_rejectall(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    memset(ic->rejection_list, 1, sizeof(ic->rejection_list));
    ic_unlock();
    return 0;
}

int ic_cmd_allowall(interrupt_controller* ic) {
    if (ic_lock() < 0) return -1;
    memset(ic->rejection_list, 0, sizeof(ic->rejection_list));
    ic_unlock();
    return 0;
}

int ic_run_command(interrupt_controller* ic, uint8_t command) {
    switch(command) {

        case IC_COMMAND_DISABLE:
            ic_cmd_disable(ic);
            return 0;
        
        case IC_COMMAND_ENABLE:
            ic_cmd_enable(ic);
            return 0;
        
        case IC_COMMAND_DROPALL:
            ic_cmd_dropall(ic);
            return 0;
        
        case IC_COMMAND_DISCARD_VEC:
            ic_cmd_discard_vector(ic);
            return 0;
        
        case IC_COMMAND_REJECT_VEC:
            ic_cmd_reject_vector(ic);
            return 0;
        
        case IC_COMMAND_ALLOW_VEC:
            ic_cmd_allow_vector(ic);
            return 0;
        
        case IC_COMMAND_REJECTALL:
            ic_cmd_rejectall(ic);
            return 0;
            
        case IC_COMMAND_ALLOWALL:
            ic_cmd_allowall(ic);
            return 0;
        
        default:
            return -1;
    }
}

int ic_read_data(interrupt_controller* ic, uint8_t* result) {
    *result = ic->data;
    return 0;
}

int ic_write_data(interrupt_controller* ic, uint8_t data) {
    ic->data = data;
    return 0;
}

int ic_isrejected(interrupt_controller* ic, uint8_t port) {
    return ic->rejection_list[port];
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