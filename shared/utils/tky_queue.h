#ifndef QUEUE_H
#define QUEUE_H

#include<stddef.h>
#include<stdbool.h>

typedef struct tky_queue tky_queue;

tky_queue*      queue_init();

void            queue_enqueue(tky_queue* q, void* data);
void*           queue_dequeue(tky_queue* q);

void            queue_enqueue_back(tky_queue* q, void* data);
void*           queue_dequeue_back(tky_queue* q);

void*           queue_peek_front(tky_queue* q);
void*           queue_peek_back(tky_queue* q);

size_t          queue_size(tky_queue* q);

void            queue_free(tky_queue* q);

bool            queue_empty(tky_queue* q);

void            queue_display(tky_queue* q);

#endif