#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>

#include "tky_queue.h"
#include "tky_llist.h"

typedef struct tky_queue {
    size_t              size;
    tky_llist*          data;
} tky_queue;

tky_queue* queue_init() {
    tky_queue* q = (tky_queue*) malloc(sizeof(tky_queue));
    q->size = 0;
    q->data = llist_init();
    return q;
}

bool queue_empty(tky_queue* q) {
    return (q->size == 0);
}

void queue_enqueue_back(tky_queue* q, void* data) {
    llist_push_back(q->data, data);
    q->size++;
}

void queue_enqueue_front(tky_queue* q, void* data) {
    llist_push_front(q->data, data);
    q->size++;
}

void* queue_dequeue_front(tky_queue* q) {
    void* d = llist_pop_front(q->data);
    if (d != NULL) q->size--;
    return d;
}

void* queue_dequeue_back(tky_queue* q) {
    void* d = llist_pop_back(q->data);
    if (d != NULL) q->size--;
    return d;
}

void* queue_peek_back(tky_queue* q) {
    return llist_get_tail(q->data);
}

void* queue_peek_front(tky_queue* q) {
    return llist_get_head(q->data);
}

size_t queue_size(tky_queue* q) {
    return q->size;
}

void queue_display(tky_queue* q) {
    llist_display(q->data);
}

void queue_free(tky_queue* q) {
    llist_destroy(q->data);
    free(q);
}
