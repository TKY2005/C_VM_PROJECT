#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include "tky_stack.h"

typedef struct tky_stack {
    int64_t     sp;
    size_t      capacity;
    void**      data;
} tky_stack;

tky_stack* stack_init() {
    tky_stack* s = (tky_stack*) malloc(sizeof(tky_stack));
    s->sp = -1;
    s->capacity = DS_INIT_SIZE;
    s->data = (void**) malloc(sizeof(void*) * DS_INIT_SIZE);
    return s;
}

int stack_empty(tky_stack* s) {
    return (s->sp < 0);
}

void stack_push(tky_stack* s, void* data) {
    if (s->sp + 1 > s->capacity) {
        s->data = (void**) realloc(s->data, s->capacity * 2);
    }
    s->data[++s->sp] = data;
}

void* stack_pop(tky_stack* s) {
    if (stack_empty(s)) return NULL;
    return s->data[s->sp--];
}

size_t stack_size(tky_stack* s) {
    return s->sp;
}

void* stack_peek(tky_stack* s) {
    if (stack_empty(s)) return NULL;
    return s->data[s->sp];
}

void stack_destroy(tky_stack* s) {
    free(s->data);
    free(s);
}
