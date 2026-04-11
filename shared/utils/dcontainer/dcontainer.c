#include "./dcontainer.h"
#include "../helpers.h"

#include<stdlib.h>

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