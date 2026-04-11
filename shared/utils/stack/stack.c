#include "./stack.h"
#include "../dynamic_list/dynamic_list.h"
#include<stdio.h>

#include<stdlib.h>

stack* mkstack() {
	stack* s = (stack*) malloc(sizeof(stack));
	s->cont = list_create();
	s->size = -1;
	return s;
}

int stack_empty(stack* s) {
	return (s->size > -1) ? 0 : 1;
}

void stack_push(stack* s, void* data){ 
	list_set(s->cont, ++s->size, data);
}
void* stack_pop(stack* s) {
	if (s->size <= -1) return NULL;
	return list_get(s->cont, s->size--);
}
void* stack_peek(stack* s) {
	if (s->size <= -1) return NULL;
	return list_get(s->cont, s->size);
}
void stack_free(stack* s) {
	list_free(s->cont, NULL);
	free(s);
}