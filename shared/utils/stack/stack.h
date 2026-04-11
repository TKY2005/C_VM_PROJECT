#ifndef STACK_H
#define STACK_H

#include "../dynamic_list/dynamic_list.h"

typedef struct {
	int size;
	dynamic_list* cont;
} stack;

stack* mkstack();

int stack_empty(stack* s);

void stack_push(stack* s, void* data);
void* stack_pop(stack* s);
void* stack_peek(stack* s);
void stack_free(stack* s);

#endif