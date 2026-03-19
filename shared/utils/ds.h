#include<stdarg.h>

#ifndef DS_H
#define DS_H

#define DS_INIT_SIZE 50

int vptoint(void* d);
float vptofloat(void* d);
char vptochar(void* d);
char* vptostr(void* d);

typedef struct {
	int capacity;
	void** data;
} dcontainer;

dcontainer mkcontainer();

int dcontainer_capacity(dcontainer d);
void dcontainer_free(dcontainer* d);

// DYNAMIC LIST FUNCTIONS //
typedef struct {
	int size;
	dcontainer cont;
} dynlst;

dynlst* mkdynlst();
void dynlst_add(dynlst* d, void* data);
int dynlst_insert(dynlst* d, int index, void* data);
int dynlst_remove(dynlst* d, int index);
void* dynlst_get(dynlst* d, int index);
void dynlst_free(dynlst* d);
int dynlst_size(dynlst* d);
int dynlst_capacity(dynlst* d);

// STACK FUNCTIONS //
typedef struct {
	int size;
	dcontainer cont;
} stack;

stack* mkstack();
void stack_push(stack* s, void* data);
void* stack_pop(stack* s);
void* stack_peek(stack* s);
void stack_free(stack* s);


// STRINGBUILDER FUNCTIONS //
typedef struct {
	int size;
	int capacity;
	char* str;
}strbuilder;

strbuilder strbuilder_init();

void strbuilder_append(strbuilder* sb, char* s);
void strbuilder_appendf(strbuilder* sb, char* s, ...);
void strbuilder_append_chr(strbuilder* sb, char c);

void strbuilder_fill(strbuilder* sb, char* f, int count);
void strbuilder_fill_chr(strbuilder* sb, char f, int count);

void strbuilder_reset(strbuilder* sb);

char* strbuilder_getstr(strbuilder* sb);
char* strbuilder_substr(strbuilder* sb, int start, int end);
int strbuilder_size(strbuilder* sb);
int strbuilder_capacity(strbuilder* sb);

void strbuilder_destroy(strbuilder* sb);


#endif
