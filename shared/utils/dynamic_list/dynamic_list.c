#include "./dynamic_list.h"
#include "../helpers.h"
#include<stddef.h>
#include<stdlib.h>
#include<string.h>

// Internal helper to double the capacity
int list_resize(dynamic_list *list) {
    size_t new_capacity = list->capacity * 2;
    void **new_data = (void**) realloc(list->data, new_capacity * sizeof(void*));
    if (!new_data)
        return 0;

    // Initialize new slots with NULL
    memset(new_data + list->capacity, 0,
           (new_capacity - list->capacity) * sizeof(void*));

    list->data = new_data;
    list->capacity = new_capacity;
    return 1;
}

// Ensure the list has at least min_capacity
static int list_ensure_capacity(dynamic_list *list, size_t min_capacity) {
    if (list->capacity >= min_capacity)
        return 1;

    while (list->capacity < min_capacity) {
        if (!list_resize(list))
            return 0;
    }
    return 1;
}

// Create a new list with DS_INITIAL_SIZE capacity
dynamic_list* list_create(void) {
    dynamic_list *list = (dynamic_list *)malloc(sizeof(dynamic_list));
    if (!list)
        return NULL;

    list->capacity = DS_INIT_SIZE;
    list->size = 0;

    list->data = (void **)calloc(list->capacity, sizeof(void *));
    if (!list->data) {
        free(list);
        return NULL;
    }

    return list;
}

// Free the list and optionally its elements
void list_free(dynamic_list *list, void (*free_func)(void*)) {
    if (!list)
        return;

    if (free_func) {
        for (size_t i = 0; i < list->size; ++i) {
            if (list->data[i])
                free_func(list->data[i]);
        }
    }

    free(list->data);
    free(list);
}

// Add element to the end
int list_add(dynamic_list *list, void *element) {
    if (!list)
        return 0;

    if (!list_ensure_capacity(list, list->size + 1))
        return 0;

    list->data[list->size++] = element;
    return 1;
}

// Insert element at index (shifts elements to the right)
int list_insert(dynamic_list *list, size_t index, void *element) {
    if (!list)
        return 0;

    size_t required = (index < list->size) ? (list->size + 1) : (index + 1);
    if (!list_ensure_capacity(list, required))
        return 0;

    if (index < list->size) {
        memmove(&list->data[index + 1],
                &list->data[index],
                (list->size - index) * sizeof(void*));
        list->size++;
    } else {
        memset(&list->data[list->size], 0,
               (index - list->size) * sizeof(void*));
        list->size = index + 1;
    }

    list->data[index] = element;
    return 1;
}

// Set element at index without shifting (expands if necessary)
int list_set(dynamic_list *list, size_t index, void *element) {
    if (!list)
        return 0;

    if (!list_ensure_capacity(list, index + 1))
        return 0;

    // Fill intermediate elements with NULL
    if (index >= list->size) {
        memset(&list->data[list->size], 0,
               (index - list->size) * sizeof(void*));
        list->size = index + 1;
    }

    list->data[index] = element;
    return 1;
}

// Get element at index
void* list_get(const dynamic_list *list, size_t index) {
    if (!list || index >= list->size)
        return NULL;
    return list->data[index];
}

// Get current size
size_t list_get_size(const dynamic_list *list) {
    return list ? list->size : 0;
}

// Get current capacity
size_t list_get_capacity(const dynamic_list *list) {
    return list ? list->capacity : 0;
}