#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<stdint.h>
#include<string.h>

#include "./ds.h"

int vptoint(void* d) {
	return (int)(uintptr_t) d;
}
float vptofloat(void* d){
	float f;
	memcpy(&f, d, sizeof(float));
	return f;
}
char vptochar(void* d) {
	char c;
	memcpy(&c, d, sizeof(char));
	return c;
	
}
char* vptostr(void* d){
	return (char*) d;
}

dcontainer mkcontainer() {
	dcontainer d;
	d.capacity = DS_INIT_SIZE;
	d.data = (void**) malloc(sizeof(void*) * d.capacity);
	return d;
}
void dcontainer_free(dcontainer* d) {
	free(d->data);
}
int dcontainer_capacity(dcontainer d) {
	return d.capacity;
}
static void dcontainer_resize(dcontainer* d) {
	d->capacity *= 2;
	d->data = (void**) realloc(d->data, sizeof(void*) * d->capacity);
}

// DYNAMIC LIST IMPLEMENTATION //

dynlst* mkdynlst() {
	dynlst* d = (dynlst*) malloc(sizeof(dynlst));
	d->cont = mkcontainer(); 
	d->size = 0;

	return d;
}

int dynlst_size(dynlst* d) {
	return d->size;
}

void dynlst_add(dynlst* d, void* data) {
	d->cont.data[d->size++] = data;
	if (d->size >= d->cont.capacity)
		dcontainer_resize(&d->cont);
}
void* dynlst_get(dynlst* d, int index) {
	if (index >= d->size) return NULL;
	else return d->cont.data[index];
}
int dynlst_insert(dynlst* d, int index, void* data) {
	if (index < 0) return -1;
	else if (index >= d->cont.capacity) {
		while (index >= d->cont.capacity) dcontainer_resize(&d->cont);
	}

	for(int i = d->size; i > index; i--) {
		d->cont.data[i] = d->cont.data[i - 1];
	}

	d->cont.data[index] = data;
	d->size++;
	return 0;
}
int dynlst_remove(dynlst* d, int index) {
	if (index < 0 || index >= d->size) return -1;

	for(int i = index; i < d->size; i++) {
		d->cont.data[i] = d->cont.data[i + 1];
	}
	d->size--;
	return 0;
}
void dynlst_free(dynlst* d) {
	dcontainer_free(&d->cont);
	free(d);
}

// STACK IMPLEMENTATION //

stack* mkstack() {
	stack* s = (stack*) malloc(sizeof(stack));
	s->cont = mkcontainer();
	s->size = -1;
	return s;
}

void stack_push(stack* s, void* data){ 
	if (s->size + 1 >= s->cont.capacity)
		dcontainer_resize(&s->cont);
	s->cont.data[++s->size] = data;
}
void* stack_pop(stack* s) {
	if (s->size <= -1) return NULL;
	else return s->cont.data[s->size--];
}
void* stack_peek(stack* s) {
	if (s->size <= -1) return NULL;
	else return s->cont.data[s->size];
}
void stack_free(stack* s) {
	dcontainer_free(&s->cont);
	free(s);
}

// STRINGBUILDER IMPLEMENTATION //
strbuilder strbuilder_init() {
	strbuilder s;
	s.size = 0;
	s.capacity = DS_INIT_SIZE;
	s.str = (char*) malloc(DS_INIT_SIZE * sizeof(char));
	s.str[0] = '\0';
	return s;
}

static void strbuilder_resize(strbuilder* sb) {
	sb->capacity *= 2;
	sb->str = (char*) realloc(sb->str, sb->capacity);
}

void strbuilder_append(strbuilder* sb, char* s) {

	for(int i = 0; s[i]; i++, sb->size++) {
		if (sb->size + 1 >= sb->capacity)
			strbuilder_resize(sb);

		sb->str[sb->size] = s[i];
	}
	sb->str[sb->size] = '\0';
}
void strbuilder_appendf(strbuilder* sb, char* s, ...) {
	
	char buff[4096] = {0};
	va_list args;
	va_start(args, s);

	vsnprintf(buff, sizeof(buff), s, args);

	va_end(args);

	for(int i = 0; buff[i]; i++, sb->size++) {
		if (sb->size + 1 >= sb->capacity)
			strbuilder_resize(sb);

		sb->str[sb->size] = buff[i];
	}
	sb->str[sb->size] = '\0';
}
void strbuilder_append_chr(strbuilder* sb, char c) {
	
	if (sb->size + 1 >= sb->capacity)
		strbuilder_resize(sb);
	sb->str[sb->size] = c;
	sb->str[++sb->size] = '\0';
}
void strbuilder_fill(strbuilder* sb, char* f, int count) {

	for(int i = 0; i < count; i++) {
		for(int j = 0; f[j]; j++, sb->size++) {
			if (sb->size + 1 >= sb->capacity)
				strbuilder_resize(sb);
			sb->str[sb->size] = f[j];
		}
	}
	sb->str[sb->size] = '\0';
}
void strbuilder_fill_chr(strbuilder* sb, char f, int count) {
	for(int i = 0; i < count; i++, sb->size++) {
		if (sb->size + 1 >= sb->capacity)
			strbuilder_resize(sb);
		sb->str[sb->size] = f;
	}
	sb->str[sb->size] = '\0';
}

void strbuilder_reset(strbuilder* sb) {
	sb->size = 0;
	sb->str[sb->size] = '\0';
}

char* strbuilder_getstr(strbuilder* sb) {
	if (sb == NULL) return NULL;
	return sb->str;
}

char* strbuilder_substr(strbuilder* sb, int start, int end) {
	if (sb == NULL) return NULL;
	if (start < 0 || end >= sb->size) return NULL;
	int size = end - start;
	char* str = (char*) malloc(size * sizeof(char));

	for(int i = start; i < end; i++) str[i] = sb->str[i];
	return str;
}

int strbuilder_size(strbuilder* sb) {
	return sb->size;
}
int strbuilder_capacity(strbuilder* sb) {
	return sb->capacity;
}

void strbuilder_destroy(strbuilder* sb) {
	free(sb->str);
	sb->capacity = 0;
	sb->size = 0;
}
