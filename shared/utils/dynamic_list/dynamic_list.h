#ifndef DYNAMIC_LIST_H
#define DYNAMIC_LIST_H

#include<stddef.h>

typedef struct {
    void **data;      // Array of void* elements
    size_t size;      // Number of elements currently stored
    size_t capacity;  // Allocated capacity
} dynamic_list;

// Creation and destruction
dynamic_list* list_create(void);
void list_free(dynamic_list *list, void (*free_func)(void*));

int list_resize(dynamic_list* l);

// Core operations
int list_add(dynamic_list *list, void *element);
int list_insert(dynamic_list *list, size_t index, void *element);
int list_set(dynamic_list *list, size_t index, void *element);

// Accessors
size_t list_get_size(const dynamic_list *list);
size_t list_get_capacity(const dynamic_list *list);
void* list_get(const dynamic_list *list, size_t index);

#endif