#ifndef STACK_H
#define STACK_H

#define DS_INIT_SIZE 50

#include<stddef.h>

typedef struct tky_stack tky_stack;

/*initalize the stack*/
tky_stack*  stack_init();
/*place an element at the top of the stack*/
void        stack_push(tky_stack* s, void* data);
/*get the element at the top of the stack and remove it from the stack*/
void*       stack_pop(tky_stack* s);
/*get the element at the top of the stack without removing it*/
void*       stack_peek(tky_stack* s);
/*checks if the stack currently holds no items*/
int         stack_empty(tky_stack* s);
/*returns the number of items currently present in the stack*/
size_t      stack_size(tky_stack* s);

void        stack_destroy(tky_stack* s);

#endif
