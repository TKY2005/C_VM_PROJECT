#ifndef VM_DISPLAY_H
#define VM_DISPLAY_H

#include<raylib.h>
#include<Memory/framebuffer.h>

#include<pthread.h>
#include<stdatomic.h>
#include<stdbool.h>

#define DISPLAY_INIT_SUCCESS 0
#define DISPLAY_INIT_FAIL -1

typedef struct vm_display {
    int width, height, refresh;
    Image display_image;
    Texture2D display_screen;
    pthread_t update_thread;
    atomic_bool close_requested;
    bool update_thread_started;
} vm_display;

typedef struct display_arg {
    vm_display* d;
    framebuffer* fb;
} display_arg;


vm_display* window_start();
void* window_update(void* arg);

int window_close(vm_display* d);

#endif