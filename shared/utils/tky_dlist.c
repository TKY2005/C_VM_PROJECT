#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stddef.h>

#include "tky_dlist.h"

typedef struct tky_dlist {
    size_t     size;
    size_t     capacity;
    void**     data;
} tky_dlist;

static void dynamic_list_resize(tky_dlist* l) {
    l->capacity *= 2;
    l->data = realloc(l->data, l->capacity * sizeof(void*));
}

size_t dlist_size(tky_dlist* l) {
    return l->size;
}
size_t dlist_capacity(tky_dlist* l) {
    return l->capacity;
}

tky_dlist* dlist_init(size_t* init_size) {
    tky_dlist* l = (tky_dlist*) malloc(sizeof(tky_dlist));
    l->size = 0;
    if (init_size) l->capacity = *init_size;
    else l->capacity = DL_INIT_SIZE;
    l->data = calloc(l->capacity, sizeof(void*));
    return l;
}

void dlist_add(tky_dlist* l, void* data) {
    if (l->size + 1 >= l->capacity) dynamic_list_resize(l);
    l->data[l->size++] = data;
}

void dlist_insert(tky_dlist* l, int index, void* data) {
    if (index < 0) return;
    while (index >= l->capacity) dynamic_list_resize(l);
    if (l->size + 1 >= l->capacity) dynamic_list_resize(l);
    memmove(&l->data[index + 1], &l->data[index], (l->capacity - index - 1) * sizeof(void*));
    l->data[index] = data;
    if (index > l->size) l->size = index + 1;
    else l->size++;
}

void dlist_set(tky_dlist* l, int index, void* data) {
    if (index < 0 || index >= l->capacity) return;
    l->data[index] = data;
}

void dlist_remove(tky_dlist* l, int index) {
    if (index < 0 || index >= l->size) return;
    memmove(&l->data[index], &l->data[index + 1], (l->size - index - 1) * sizeof(void*));
    l->data[l->size - 1] = NULL;
    l->size--;
}

void* dlist_get(tky_dlist* l, int index) {
    if (index >= l->capacity) return NULL;
    else return l->data[index];
}

void dlist_match_capacity(tky_dlist* l) {
    l->size = l->capacity;
}

void dlist_free(tky_dlist* l) {
    free(l->data);
    free(l);
}
