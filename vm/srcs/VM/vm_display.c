#include<VM/vm_display.h>
#include<VM/vm_settings.h>
#include<Memory/framebuffer.h>
#include<VM/vm.h>
#include<raylib.h>

#include<stdlib.h>
#include<stdio.h>

vm_display* window_start() {
    vm_display* d = (vm_display*) calloc(1, sizeof(vm_display));
    if (d == NULL) return NULL;
    d->width = vm_conf.displayW;
    d->height = vm_conf.displayH;
    d->refresh = vm_conf.displayR;
    atomic_init(&d->close_requested, false);
    return d;
}

void* window_update(void* arg) {
    if (arg == NULL) {
        fprintf(stderr, "Display thread received invalid arguments.\n");
        return NULL;
    }
    display_arg* a = (display_arg*) arg;
    framebuffer* fb = a->fb;
    vm_display* d = a->d;
    if (fb == NULL || d == NULL) {
        fprintf(stderr, "Display thread received uninitialized state.\n");
        return NULL;
    }

    if (d->width <= 0 || d->height <= 0 ||
        (size_t)d->width > SIZE_MAX / (size_t)d->height / sizeof(uint32_t)) {
        fprintf(stderr, "Invalid display dimensions: %d x %d.\n", d->width, d->height);
        return NULL;
    }
    size_t texture_size = (size_t)d->width * (size_t)d->height * sizeof(uint32_t);
    if (texture_size > fb->size) {
        fprintf(stderr, "Display frame exceeds the VM framebuffer size.\n");
        return NULL;
    }

    InitWindow(d->width, d->height, "TKY PC emulator");
    if (!IsWindowReady()) {
        fprintf(stderr, "Unable to initialize display window.\n");
        return NULL;
    }
    d->display_image = GenImageColor(d->width, d->height, BLANK);
    d->display_screen = LoadTextureFromImage(d->display_image);
    UnloadImage(d->display_image);
    if (!IsTextureValid(d->display_screen)) {
        fprintf(stderr, "Unable to initialize display texture.\n");
        CloseWindow();
        return NULL;
    }
    SetTargetFPS(d->refresh);

    while (!atomic_load(&d->close_requested) && !WindowShouldClose()) {
        if (framebuffer_lock(fb) != 0) {
            break;
        }
        UpdateTexture(d->display_screen, fb->mem);
        if (framebuffer_unlock(fb) != 0) {
            break;
        }
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(d->display_screen, 0, 0, WHITE);
        EndDrawing();
    }
    UnloadTexture(d->display_screen);
    CloseWindow();
    return NULL;
}


int window_close(vm_display* d) {
    if (d == NULL) return 0;
    atomic_store(&d->close_requested, true);
    if (d->update_thread_started) {
        int result = pthread_join(d->update_thread, NULL);
        if (result != 0) {
            fprintf(stderr, "Unable to join display thread (error %d).\n", result);
            return -1;
        }
    }
    free(d);
    return 0;
}